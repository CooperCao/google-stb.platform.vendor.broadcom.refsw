/*
 * BME test implementation.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id$
 */
#include <osl.h>
#include "bme.h"


/* BME transfer test */
void bme_transfer(bme_params_t *bme_params, volatile arm_stat_t *stat)
{
	uint32 count = 0;

	if (bme_params->src0) {
		BME_TRANSFER_REQ0(bme_params->src0, bme_params->dst0, bme_params->len0);
	}
	else {
		BME_ZEROSET_REQ0(bme_params->dst0, bme_params->len0);
	}

	if (bme_params->src1) {
		BME_TRANSFER_REQ1(bme_params->src1, bme_params->dst1, bme_params->len1);
	}
	else {
		BME_ZEROSET_REQ1(bme_params->dst1, bme_params->len1);
	}

	do {
		count ++;
	} while (BME_STAT_BUSY0);
	stat->bme_cmplt_cnt[0] = count;

	do {
		count ++;
	} while (BME_STAT_BUSY1);
	stat->bme_cmplt_cnt[1] = count;
}
