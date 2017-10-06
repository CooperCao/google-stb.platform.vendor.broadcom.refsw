/*
 * Simulated NVRAM variables
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
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>

#ifdef	BCMDBG
#define NVR_MSG(x)	printf x
#else
#define NVR_MSG(x)
#endif

#define NVR_MEM_MAX	1024

char nvr_mem[NVR_MEM_MAX];
char *nvr_memptr;

static void *
nvr_alloc(int len)
{
	char *ptr = nvr_memptr;
	len = (len + 3) & ~3;
	ASSERT(ptr + len <= nvr_mem + NVR_MEM_MAX);
	nvr_memptr += len;
	return ptr;
}

#define nvr_free(ptr)		((void)(ptr))	/* Leak, but don't care for simulation */

struct nvr_var {
	char *name, *value;
	struct nvr_var *next;
};

struct nvr_var *nvram;
struct nvr_var *defvars;

static void nvr_set(struct nvr_var **vars, const char *name, const char *value);
static char *nvr_get(struct nvr_var **vars, const char *name);

int
BCMINITFN(nvram_init)(void *sb)
{
	nvr_memptr = nvr_mem;

	nvr_set(&nvram, "wl0_ssid", "rtesim");
	nvr_set(&nvram, "wl0_channel", "6");
	nvr_set(&nvram, "wl0_gmode", "1");

	nvr_set(&defvars, "il0macaddr", "00:11:22:33:44:55");
	nvr_set(&defvars, "boardtype", "0xffff");
	nvr_set(&defvars, "boardrev", "0x10");
	nvr_set(&defvars, "boardflags", "8");
	nvr_set(&defvars, "aa0", "3");

	return 0;
}

void
BCMINITFN(nvram_exit)(void *sb)
{
}

static char *
nvr_get(struct nvr_var **vars, const char *name)
{
	struct nvr_var *v;

	for (v = *vars; v; v = v->next)
		if (strcmp(v->name, name) == 0)
			return v->value;

	return NULL;
}

/* Use NULL value to delete any entry */
static void
nvr_set(struct nvr_var **vars, const char *name, const char *value)
{
	struct nvr_var *v;

	/* Delete if already present */
	while ((v = *vars) != NULL)
		if (strcmp(v->name, name) == 0) {
			(*vars) = v->next;
			nvr_free(v->name);
			nvr_free(v->value);
			nvr_free(v);
		} else
			vars = &(*vars)->next;


	if (value != NULL) {
		v = nvr_alloc(sizeof(struct nvr_var));
		v->name = nvr_alloc(strlen(name) + 1);
		strcpy(v->name, name);
		v->value = nvr_alloc(strlen(value) + 1);
		strcpy(v->value, value);
		v->next = (*vars);
		(*vars) = v;
	}
}

char *
BCMINITFN(nvram_get)(const char *name)
{
	char *v;

	if ((v = nvr_get(&nvram, name)))
		return v;

	if ((v = nvr_get(&defvars, name))) {
		NVR_MSG(("%s: variable %s defaulted to %s\n", __FUNCTION__, name, v));
		return v;
	}

	return NULL;
}

int
BCMINITFN(nvram_set)(const char *name, const char *value)
{
	printf("nvram_set(%s,%s)\n", name, value);
	nvr_set(&nvram, name, value);
	return 0;
}

int
BCMINITFN(nvram_unset)(const char *name)
{
	nvr_set(&nvram, name, NULL);
	return 0;
}

int
BCMINITFN(nvram_commit)(void)
{
	return 0;
}

int
BCMINITFN(nvram_getall)(char *buf, int count)
{
	struct nvr_var *v;

	for (v = nvram; v; v = v->next) {
		int len = strlen(v->name) + 1 + strlen(v->value) + 1;
		if (count < len)
			return BCME_BUFTOOSHORT;
		len = strlen(v->name);
		strcpy(buf, v->name);
		buf += len;
		count -= len;
		*buf++ = '=';
		count--;
		len = strlen(v->value);
		strcpy(buf, v->value);
		buf += len + 1;
		count -= len + 1;
	}

	if (count < 1)
		return BCME_BUFTOOSHORT;
	*buf = '\0';

	return 0;
}
