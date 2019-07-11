/*
 * CGI helper functions
 *
 * Copyright (C) 2018, Broadcom Corporation. All Rights Reserved.
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
 * $Id: cgi.c 707888 2017-06-29 06:28:21Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifdef BCMDBG
#include <assert.h>
#else
#define assert(a)
#endif

#if defined(linux)
/* Use SVID search */
#define __USE_GNU
#include <search.h>
#elif defined(__ECOS)
#include <search.h>
#define table htable
#endif

#ifdef DISABLE_HSEARCH
typedef struct ts_entry
{
    char *key;
    void *data;
}
TS_ENTRY;
#endif

/* CGI hash table */
#ifndef DISABLE_HSEARCH
static struct hsearch_data htab;
#else
static void *troot = NULL;
#endif
static int htab_count;

#ifdef DISABLE_HSEARCH
static int
compare(const void *pa, const void *pb)
{
	TS_ENTRY *ts1, *ts2;

	ts1 = (TS_ENTRY *)pa;
	ts2 = (TS_ENTRY *)pb;

	return strcmp(ts1->key, ts2->key);
}

static void ts_free(void *p)
{
	if (p) {
		free(p);
	}
}
#endif

static void
unescape(char *s)
{
	unsigned int c;

	while ((s = strpbrk(s, "%+"))) {
		/* Parse %xx */
		if (*s == '%') {
			sscanf(s + 1, "%02x", &c);
			*s++ = (char) c;
			memmove(s, s + 2, strlen(s) - 1);
		}
		/* Space is special */
		else if (*s == '+')
			*s++ = ' ';
	}
}

char *
get_cgi(char *name)
{
#ifndef DISABLE_HSEARCH
	ENTRY e, *ep;

	if (!htab.table)
		return NULL;

	e.key = name;
	hsearch_r(e, FIND, &ep, &htab);

	return ep ? ep->data : NULL;
#else
	TS_ENTRY e, *ep;
	void *result;

	e.key = name;
	result = tfind((void*)&e, &troot, compare);

	if (result == NULL) {
		ep = NULL;
	} else {
		ep = *(TS_ENTRY **)result;
	}

	return ep ? ep->data : NULL;
#endif
}

void
set_cgi(char *name, char *value)
{
#ifndef DISABLE_HSEARCH
	ENTRY e, *ep;

	if (!htab.table)
		return;

	e.key = name;
	hsearch_r(e, FIND, &ep, &htab);
	if (ep)
		ep->data = value;
	else {
		e.data = value;
		hsearch_r(e, ENTER, &ep, &htab);
		htab_count++;
	}
	assert(ep);
#else
	TS_ENTRY e, *ep;
	void *result;

	e.key = name;
	result = tfind((void*)&e, &troot, compare);
	if (result == NULL) {
		ep = (TS_ENTRY *)malloc(sizeof(TS_ENTRY));
		ep->key = name;
		ep->data = value;
		tsearch((void*)ep, &troot, compare);
		htab_count++;
	} else {
		ep = *(TS_ENTRY **)result;
		ep->data = value;
	}
#endif
}

void
init_cgi(char *query)
{
	int len, nel;
	char *q, *name, *value;

	htab_count = 0;

	/* Clear variables */
	if (!query) {
#ifndef DISABLE_HSEARCH
		hdestroy_r(&htab);
#else
		tdestroy(troot, ts_free);
#endif
		return;
	}

	/* Parse into individual assignments */
	q = query;
	len = strlen(query);
	nel = 1;
	while (strsep(&q, "&;"))
		nel++;
#ifndef DISABLE_HSEARCH
	hcreate_r(nel, &htab);
#endif

	for (q = query; q < (query + len);) {
		/* Unescape each assignment */
		unescape(name = value = q);

		/* Skip to next assignment */
		for (q += strlen(q); q < (query + len) && !*q; q++);

		/* Assign variable */
		name = strsep(&value, "=");
		if (value) {
			set_cgi(name, value);
		}
	}
}

int
count_cgi()
{
	return htab_count;
}
