/**
 * @file
 * @brief
 * Ultra Low Power [ulp] base functions header
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

#ifndef _ULP_H_
#define _ULP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Include Files ---------------------------------------------------- */
#include <osl.h>
#include <siutils.h>

#include <sbchipc.h>

/* ---- Constants and Types ---------------------------------------------- */

/* ulp dbg macro */
#define ULP_DBG(x)
#define ULP_ERR(x) printf x
/* enable following code to prhex
#define ULP_DBG_ON (1)
*/
typedef struct ulp_ext_info ulp_ext_info_t;

/* ULP phases */
enum {
	ULP_PHASE1 = 0, /* phase 1 is storage of data used only in DS0 */
	ULP_PHASE2 = 1, /* phase 2 is storage of data used in both DS0 and DS1 modes */
	ULP_PHASE_LAST = 2
};

typedef struct shmdefs_struct shmdefs_t;
typedef struct p2_handle {
	void *wlc_hw;
	const shmdefs_t *shmdefs;		/* Pointer to Auto-SHM strucutre */
} p2_handle_t;

/* module ID's registering for a cubby in ulp buffer. Note that the sequence
 * is significant here. ulp_recreate() calls in this order than the attach
 * sequence. This is to make sure that, we can control the ulp sequences
 * properly.
 */
typedef enum {
	ULP_MODULE_ID_RSVD = 0,
	ULP_MODULE_ID_PMU = 1,
	ULP_MODULE_ID_SDIO = 2,
	ULP_MODULE_ID_WLFC = 3,
	ULP_MODULE_ID_WLC_BSSCFG = 4,
	ULP_MODULE_ID_WLC_PM = 5,
	ULP_MODULE_ID_WLC_SUP = 6,
	ULP_MODULE_ID_WLC_ULP = 7,
	ULP_MODULE_ID_KM = 8,
	ULP_MODULE_ID_PHY_RADIO = 9,
	ULP_MODULE_ID_WLC_ULP_BTCX = 10,
	ULP_MODULE_ID_WLC_ULP_LTECX = 11,
	ULP_MODULE_ID_BMAC = 12,
	ULP_MODULE_ID_LAST = 13
} ulp_module_id_t;

#define	ULP_SDIO_CMD_PIN	CC_GCI_GPIO_8

/* Note that 0 is reserved for module id. */
#define ULP_MAX_P1_MODULES	(ULP_MODULE_ID_LAST - 1)
#define ULP_MAX_P2_MODULES	(5)

/* flags: cubby type defines */
/* Static modules have the same data stored in the cubby irrespective of open and security modes */
#define MODCBFL_CTYPE_STATIC	(0x00000000)

/* by default static, if the cubby needs to be dynamic, use this flag */
/* dynamic modules cubby content differs based on association type - open/secure */
#define MODCBFL_CTYPE_DYNAMIC	(0x00000001)

#define ULP_MIN_RES_MASK	0x00000001

#define ULP_SR_RESOURCE		23
#define ULP_SR_UPDOWNTIME	0

#define DS1_43012A0_FAST_PWRUP_DELAY	2464 /* in us */


#ifdef BCMULP
	extern bool _bcmulp;
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define BCMULP_ENAB() (_bcmulp)
	#elif defined(BCMULP_DISABLED)
		#define BCMULP_ENAB()	(0)
	#else
		#define BCMULP_ENAB()	(1)
	#endif
#else
	#define BCMULP_ENAB()		(0)
#endif /* BCMULP */

/* call back types */
typedef int (*ulp_enter_pre_ulpucode_fn_t)(void *handle, ulp_ext_info_t *einfo,
	uint8 *cache_data);
typedef int (*ulp_enter_post_ulpucode_fn_t)(void *handle, ulp_ext_info_t *einfo);
typedef int (*ulp_exit_fn_t)(void *handle, uint8 *cache_data,
	uint8 *p2_cache_data);
typedef uint (*ulp_cubby_sz_fn_t)(void *handle, ulp_ext_info_t *einfo);
typedef int (*ulp_recreate_fn_t)(void *handle, ulp_ext_info_t *einfo,
	uint8 *cache_data, uint8 *p2_cache_data);
typedef int (*ulp_p2_retriv_fn_t)(void *handle, osl_t *osh,
	uint8 *p2_cache_data);

/* public context for phase1 */
typedef struct ulp_p1_module_pubctx {
	uint32			flags;	/* flags: see MODCBFL_xxx defines */
	ulp_enter_pre_ulpucode_fn_t	fn_enter_pre_ulpucode;	/* fn cb for enter before
		* switching to ulp ucode. Used to populate data to be stored in cached_data.
		*/
	ulp_exit_fn_t		fn_exit;	/* fn for exit */
	ulp_cubby_sz_fn_t	fn_cubby_size;	/* fn for getting cubby size */
	ulp_recreate_fn_t	fn_recreate;	/* fn for recreate */
	ulp_enter_post_ulpucode_fn_t	fn_enter_post_ulpucode;	/* fn cb for enter AFTER
		* switching to ulp ucode. Used to write shm's which will be used by DS1 ucode.
		*/
} ulp_p1_module_pubctx_t;

/* public context for phase2 */
typedef struct ulp_p2_module_pubctx {
	uint32 size;	/* size. Note that unlike p1, p2 supports fixed size */
	ulp_p2_retriv_fn_t fn_p2_retriv; /* p2 retrieval function */
} ulp_p2_module_pubctx_t;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */
extern int ulp_p1_module_register(ulp_module_id_t mid, const ulp_p1_module_pubctx_t *mod_cb,
	void *hdl);
extern int ulp_enter_pre_ulpucode_switch(ulp_ext_info_t *einfo);
extern uint ulp_p1_get_max_box_size(ulp_ext_info_t *einfo);
extern int ulp_recreate(ulp_ext_info_t *einfo);
extern void ulp_cleanup_cache(void);
extern int ulp_p2_module_register(ulp_module_id_t mid, const ulp_p2_module_pubctx_t *ctx);
extern int ulp_p2_retrieve(void *ctx);
extern void ulp_enter(si_t *sih, osl_t *osh, ulp_ext_info_t *einfo);
extern void ulp_exit(si_t *sih, osl_t *osh);
extern void ulp_write_driver_shmblk(si_t *sih, uint coreunit, uint offset, const void* buf,
	int len);
extern void ulp_read_driver_shmblk(si_t *sih, uint coreunit, uint offset, void* buf,
	int len);
extern uint32 ulp_mac_ulp_features_support(si_t *sih);
extern uint32 ulp_mac_ulp_features_default(si_t *sih);
extern int ulp_module_init(osl_t *osh, si_t *sih);

#ifdef __cplusplus
}
#endif
#endif /* _ulp_h */
