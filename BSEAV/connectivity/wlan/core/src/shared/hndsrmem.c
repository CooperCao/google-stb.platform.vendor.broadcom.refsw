/*
 * SR memory management implementation
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
 * $Id: hndsrmem.c $
 */
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hndsrmem.h>
#include <siutils.h>
#include <bcmutils.h>
#include <saverestore.h>

#ifdef SRMEM_DISABLED
bool _srmem = FALSE;
#else
bool _srmem = TRUE;
#endif

struct srmem_info {
	si_t *sih;
	uchar *start;
	uchar *end;
	uchar *free;
	uint remain_len;
	uint inused;
	bool enable;
};

srmem_t *srmem = NULL;

static void
BCMATTACHFN(srmem_reset)(srmem_t *srmem)
{
	ASSERT(srmem != NULL);

	srmem->start = (uint8 *)hnd_get_memsize();
	srmem->free = srmem->start;
	srmem->remain_len = si_socram_srmem_size(srmem->sih) - SR_ASMSIZE;
	srmem->end = srmem->start + srmem->remain_len;
	srmem->enable = TRUE;
}

void *
srmem_alloc(srmem_t *srmem, uint size)
{
	uint tot = ROUNDUP(size, sizeof(int));

	ASSERT(srmem != NULL);

	if (tot <= srmem->remain_len)
	{
		osl_t *osh = si_osh(srmem->sih);
		void *srbuf = (void *)srmem->free;
		void *p = PKTGETHEADER(osh, srbuf, size, lbuf_basic);
		if (p)
		{
			srmem->free += tot;
			srmem->remain_len -= tot;
			srmem->inused++;
			return p;
		}
	}
	return NULL;
}

void
srmem_enable(srmem_t *srmem, bool enable)
{
	ASSERT(srmem != NULL);

	srmem->enable = enable;
}

void
srmem_inused(srmem_t *srmem, struct lbuf *p, bool inused)
{
	uchar * head;

	ASSERT(srmem != NULL);

#if defined(HNDLBUFCOMPACT)
	head = (uchar*) (uintptr)p->head_off;
#else  /* ! HNDLBUFCOMPACT */
	head = p->head;
#endif /* ! HNDLBUFCOMPACT */

	if (head >= srmem->start && head < srmem->end)
	{
		if (inused)
		{
			srmem->inused++;
		}
		else
		{
			srmem->inused--;
		}
	}
}

/*
 * About to enter save mode, add any pre-processing here
 */
static bool
srmem_save(void *arg)
{
	bool sr_enterok = TRUE;
	srmem_t *srmem = arg;

	ASSERT(srmem != NULL);

	if (srmem->enable)
		sr_enterok = (srmem->inused == 0) ? TRUE : FALSE;

	return sr_enterok;
}

/* Initialize the save-restore memory management */
void
BCMATTACHFN(srmem_init)(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	srmem = (void *)MALLOC(osh, sizeof(srmem_t));
	ASSERT(srmem);

	bzero(srmem, sizeof(srmem_t));

	srmem->sih = (void *)sih;
	srmem_reset(srmem);

	sr_register_save(sih, srmem_save, (void *)srmem);
}

void
BCMATTACHFN(srmem_deinit)(srmem_t *srmem)
{
	if (srmem)
	{
		srmem_reset(srmem);
	}
}
