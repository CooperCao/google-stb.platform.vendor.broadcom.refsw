/*
 * ATE restruct.
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
#ifndef _wl_ate_h_
#define _wl_ate_h_

#include <siutils.h>
#include <rte_dev.h>
#include <wlc_phy_hal.h>


/* Buffer of size WLC_SAMPLECOLLECT_MAXLEN (=10240 for 4345a0 ACPHY)
 * gets copied to this, multiple times
 */
#define ATE_SAMPLE_COLLECT_BUFFER_SIZE	(60*1024)

/* Externally used ATE  data structures */
typedef struct _ate_params {
	void*	wl;
	wlc_hw_info_t * wlc_hw;
	si_t*	sih;
	uint8	gpio_input;
	uint8	gpio_output;
	bool	cmd_proceed;
	uint16	cmd_idx;
	bool	ate_cmd_done;
} ate_params_t;

enum {
	GMULT_LPF,
	GMULT_ADC,
	RCCAL_DACBUF,
	CURR_RADIO_TEMP,
	RCAL_VALUE
};
/* Buffer defn for storing various register values */
struct ate_buffer_regval_st {
	uint32 gmult_lpf;       /* RCCal lpf result */
	uint32 gmult_adc;       /* RCCal adc result */
	uint32 rccal_dacbuf;    /* RCCal dacbuf result */
	uint32 curr_radio_temp; /* The current radio temperature */
	uint32 rccal_dacbuf_cmult; /* The current radio temperature */
	uint32 rcal_value[PHY_CORE_MAX]; /* Rcal value per core */
};
typedef struct ate_buffer_regval_st ate_buffer_regval_t;
extern ate_buffer_regval_t ate_buffer_regval[];
/* Buffer for storing various register values */
extern ate_params_t ate_params;
extern char ate_buffer_sc[];
extern uint32 ate_buffer_sc_size;

/* Externally used ATE functions */
void wl_ate_cmd_proc(void);
void wl_ate_init(si_t *sih, hnd_dev_t *bcmwl);
extern void wl_ate_set_buffer_regval(uint8 type, uint32 value, int core, uint sliceidx, uint chip);

#endif /* _wl_ate_h_ */
