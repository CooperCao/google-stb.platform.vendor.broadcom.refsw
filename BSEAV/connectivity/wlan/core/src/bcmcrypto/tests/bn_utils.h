
/* bn_utils.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: $
 *
 *   <<Broadcom-WL-IPTag/Open:>>
 */

#include <bcm_bn.h>


void Print(char* title, const bn_t* b);

bn_t* get_rand_key(bn_ctx_t* bnx);

void set_rand_key(bn_t* rKey);

void shalom_clear();

void* shalom_alloc(void* ctx, int size);

void shalom_free(void* ctx, void *pp, int size);

bn_t* shalom_rand(void* ctx, bn_ctx_t *bnx, int bit_len);
