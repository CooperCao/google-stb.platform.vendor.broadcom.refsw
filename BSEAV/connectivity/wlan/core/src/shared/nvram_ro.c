/*
 * Read-only support for NVRAM on flash and otp.
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <sbchipc.h>
#include <bcmsrom.h>
#include <bcmotp.h>
#include <bcmdevs.h>
#include <sflash.h>
#include <hndsoc.h>
#if defined(NVRAM_FLASH) || defined(BCMNVRAMR)
#include <bcmsrom_tbl.h>
#endif /* NVRAM_FLASH */

#ifdef BCMDBG_ERR
#define NVR_MSG(x) printf x
#else
#define NVR_MSG(x)
#endif	/* BCMDBG_ERR */

#define NUM_VSIZES 16
typedef struct _vars {
	struct _vars *next;
	int bufsz;		/* allocated size */
	int size;		/* actual vars size */
	char *vars;
	uint16 vsizes[NUM_VSIZES];
} vars_t;


#define	VARS_T_OH	sizeof(vars_t)

static vars_t *vars = NULL;

#ifdef NVRAM_FLASH
static void read_wlan_flash(si_t *sih, osl_t *osh,  char **nvramp, int *nvraml);
static uint8 soc_nvram_calc_crc(struct nvram_header *nvh);
#endif /* NVRAM_FLASH */

#if defined(NVRAM_FLASH) || defined(BCMNVRAMR)
#define FILE_PATHLEN	512
#define NVRAM_FILE_OEM_ANDROID "/hwcfg/nvm.txt"
#define NVRAM_FILE_DEFAULT "nvram.txt"
extern char nvram_path[];
extern char board_nvram_path[];

static int flash_update_nvbuf(osl_t *osh, char *nvram_buf, int *nvramlen, char *flshdev_data, char *pvar, char *pvalue);
static int flash_add_new_var(osl_t *osh, char *nvram_buf, int *nvramlen, char *pvar, char *pvalue);
static int flash_modify_var(osl_t *osh, char *nvram_buf,int *nvramlen, char *pvar, char *pvalue);
static int get_pavar_tbl(const pavars_t **pav, uint32 paparambwver, uint32 sromrev);
static int pavar_check(osl_t *osh, char *nvram_buf, char *flshdev_data, int32 *value_each_subband, int32 *panum, bool *ispa, char *pvar, char *pvalue);
static int update_po_value(osl_t *osh, char *nvram_buf, char *pnvvalue, int *nvramlen, uint16 *podata, int32 ponum, int32 value_each_subband);
static int remove_devpath_var(si_t *sih, char **pvar);
static int board_nvram_update(si_t *sih, osl_t *osh, char *nvram_buf, int *nvramlen, char *board_tmpbuf, char *board_buf, int board_buflen);
#endif /* NVRAM_FLASH || BCMNVRAMR */

#if defined(BCMNVRAMR)
static int nvram_file_init(void* sih);
static char* get_default_nvram_filename(void);
static char* get_board_nvram_filename(void);
static int initvars_file(si_t *sih, osl_t *osh, char *nvram_file, char **nvramp, int *nvraml);
#endif

static char *findvar(char *vars_arg, char *lim, const char *name);

extern void nvram_get_global_vars(char **varlst, uint *varsz);
static char *nvram_get_internal(const char *name);
static int nvram_getall_internal(char *buf, int count);

static void
#if defined(WLTEST) || defined(BCMDBG_DUMP)
sortvars(si_t *sih, vars_t *new)
#else
	BCMATTACHFN(sortvars)(si_t *sih, vars_t *new)
#endif
{
	osl_t *osh = si_osh(sih);
	char *s = new->vars;
	int i;
	char *temp;
	char *c, *cend;
	uint8 *lp;

	/*
	 * Sort the variables by length.  Everything len NUM_VISZES or
	 * greater is dumped into the end of the area
	 * in no particular order.  The search algorithm can then
	 * restrict the search to just those variables that are the
	 * proper length.
	 */

	/* get a temp array to hold the sorted vars */
	/* freed in same function */
	temp = MALLOC_NOPERSIST(osh, NVRAM_ARRAY_MAXSIZE + 1);
	if (!temp) {
		NVR_MSG(("Out of memory for malloc for NVRAM sort"));
		return;
	}

	c = temp;
	lp = (uint8 *) c++;

	/* Mark the var len and exp len as we move to the temp array */
	while ((s < (new->vars + new->size)) && *s) {
		uint8 len = 0;
		char *start = c;
		while ((*s) && (*s != '=')) {  /* Scan the variable */
			*c++ = *s++;
			len++;
		}

		/* ROMS have variables w/o values which we skip */
		if (*s != '=') {
			c = start;

		} else {
			*lp = len; 		/* Set the len of this var */
			lp = (uint8 *) c++;
			s++;
			len = 0;
			while (*s) { 		/* Scan the expr */
				*c++ = *s++;
				len++;
			}
			*lp = len; 		/* Set the len of the expr */
			lp = (uint8 *) c++;
			s++;
		}
	}

	cend = (char *) lp;

	s = new->vars;
	for (i = 1; i <= NUM_VSIZES; i++) {
		new->vsizes[i - 1] = (uint16) (s - new->vars);
		/* Scan for all variables of size i */
		for (c = temp; c < cend;) {
			int len = *c++;
			if ((len == i) || ((i == NUM_VSIZES) && (len >= NUM_VSIZES))) {
				/* Move the variable back */
				while (len--) {
					*s++ = *c++;
				}

				/* Get the length of the expression */
				len = *c++;
				*s++ = '=';

				/* Move the expression back */
				while (len--) {
					*s++ = *c++;
				}
				*s++ = 0;	/* Reinstate string terminator */

			} else {
				/* Wrong size - skip to next in temp copy */
				c += len;
				c += *c + 1;
			}
		}
	}

	MFREE(osh, temp, NVRAM_ARRAY_MAXSIZE + 1);
}

#if defined(FLASH)
/** copy flash to ram */
static void
BCMATTACHFN(get_flash_nvram)(si_t *sih, struct nvram_header *nvh)
{
	osl_t *osh;
	uint nvs, bufsz;
	vars_t *new;

	osh = si_osh(sih);

	nvs = R_REG(osh, &nvh->len) - sizeof(struct nvram_header);
	bufsz = nvs + VARS_T_OH;

	/* nvram data freed in si_detach */
	if ((new = (vars_t *)MALLOC_NOPERSIST(osh, bufsz)) == NULL) {
		NVR_MSG(("Out of memory for flash vars\n"));
		return;
	}
	new->vars = (char *)new + VARS_T_OH;

	new->bufsz = bufsz;
	new->size = nvs;
	new->next = vars;
	vars = new;
	sortvars(sih, new);

#ifdef BCMJTAG
	if (BUSTYPE(sih->bustype) == JTAG_BUS) {
		uint32 *s, *d;
		uint sz = nvs;

		s = (uint32 *)(&nvh[1]);
		d = (uint32 *)new->vars;

		ASSERT(ISALIGNED((uintptr)s, sizeof(uint32)));
		ASSERT(ISALIGNED((uintptr)d, sizeof(uint32)));

		while (sz >= sizeof(uint32)) {
			*d++ = ltoh32(R_REG(osh, s++));
			sz -= sizeof(uint32);
		}
		if (sz) {
			union {
				uint32	w;
				char	b[sizeof(uint32)];
			} data;
			uint i;
			char *dst = (char *)d;

			data.w =  ltoh32(R_REG(osh, s));
			for (i = 0; i < sz; i++)
				*dst++ = data.b[i];
		}
	} else
#endif	/* BCMJTAG */
		bcopy((char *)(&nvh[1]), new->vars, nvs);

	NVR_MSG(("%s: flash nvram @ %p, copied %d bytes to %p\n", __FUNCTION__,
	         nvh, nvs, new->vars));
}
#endif /* FLASH */

#if defined(BCMHOSTVARS)
#if defined(BCMHOSTVARS)
extern char *_vars;
extern uint _varsz;
#endif
extern uint8 embedded_nvram[];
#endif	

int
BCMATTACHFN(nvram_init)(void *si)
{
#if defined(FLASH)
	uint idx;
	chipcregs_t *cc;
	si_t *sih;
	osl_t *osh;
	void *oh;
	struct nvram_header *nvh = NULL;
	uintptr flbase;
	struct sflash *info;
	uint32 cap = 0, off, flsz;
#endif /* FLASH */

	/* Make sure we read nvram in flash just once before freeing the memory */
	if (vars != NULL) {
		NVR_MSG(("nvram_init: called again without calling nvram_exit()\n"));
		return 0;
	}

#if defined(FLASH)
	sih = (si_t *)si;
	osh = si_osh(sih);

	/* Check for flash */
	idx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc);

	flbase = (uintptr)OSL_UNCACHED((void *)SI_FLASH2);
	flsz = 0;
	cap = R_REG(osh, &cc->capabilities);

	switch (cap & CC_CAP_FLASH_MASK) {
	case PFLASH:
		flsz = SI_FLASH2_SZ;
		break;

	case SFLASH_ST:
	case SFLASH_AT:
		if ((info = sflash_init(sih, cc)) == NULL)
			break;
		flsz = info->size;
		break;

	case FLASH_NONE:
	default:
		break;
	}

	/* If we found flash, see if there is nvram there */
	if (flsz != 0) {
		off = FLASH_MIN;
		nvh = NULL;
		while (off <= flsz) {
			nvh = (struct nvram_header *)(flbase + off - NVRAM_SPACE);
			if (R_REG(osh, &nvh->magic) == NVRAM_MAGIC)
				break;
			off <<= 1;
			nvh = NULL;
		};

		if (nvh != NULL)
			get_flash_nvram(sih, nvh);
	}

	/* Check for otp */
	if ((oh = otp_init(sih)) != NULL) {
		uint sz = otp_size(oh);
		uint bufsz = sz + VARS_T_OH;
		/* can be freed in same function */
		vars_t *new = (vars_t *)MALLOC_NOPERSIST(osh, bufsz);
		if (new != NULL)
			new->vars = (char *)new + VARS_T_OH;
		if (new == NULL) {
			NVR_MSG(("Out of memory for otp\n"));
		} else if (otp_nvread(oh, new->vars, &sz)) {
			NVR_MSG(("otp_nvread error\n"));
			MFREE(osh, new, bufsz);
		} else {
			new->bufsz = bufsz;
			new->size = sz;
			new->next = vars;
			vars = new;
			sortvars(sih, new);
		}
	}

	/* Last, if we do have flash but no regular nvram was found in it,
	 * try for embedded nvram.
	 * Note that since we are doing this last, embedded nvram will override
	 * otp, a change from the normal precedence in the designs that use
	 * the full read/write nvram support.
	 */
	if ((flsz != 0) && (nvh == NULL)) {
		nvh = (struct nvram_header *)(flbase + 1024);
		if (R_REG(osh, &nvh->magic) == NVRAM_MAGIC)
			get_flash_nvram(sih, nvh);
		else {
			nvh = (struct nvram_header *)(flbase + 4096);
			if (R_REG(osh, &nvh->magic) == NVRAM_MAGIC)
				get_flash_nvram(sih, nvh);
		}
	}

	/* All done */
	si_setcoreidx(sih, idx);
#endif /* FLASH */

#if defined(BCMHOSTVARS)
	/* Honor host supplied variables and make them global */
	if (_vars != NULL && _varsz != 0)
		nvram_append(si, _vars, _varsz);
#endif	

#if defined(BCMNVRAMR)
	if ((BUSTYPE(((si_t *)si)->bustype) == PCI_BUS) ||
		(BUSTYPE(((si_t *)si)->bustype) == SI_BUS)) {

		if (nvram_file_init(si) != 0)
			return BCME_ERROR;
	}
#endif /* BCMNVRAMR */

	return 0;
}

#if defined(NVRAM_FLASH) || defined(BCMNVRAMR)
static int board_nvram_update(si_t *sih, osl_t *osh, char *nvram_buf, int *nvramlen, char *board_tmpbuf, char *board_buf, int board_buflen)
{
	char *pvar = NULL;
	char *pvalue = NULL;
	int err = BCME_OK;
	int i = 0;

	i = 0;
	while (i < board_buflen) {
		pvar = board_tmpbuf+i;
		pvalue = NULL;
		if ( pvar && ((i += strlen(pvar)+1) < board_buflen)) {
			if ((pvalue = strstr(pvar, "="))) {
				*pvalue = '\0';
				pvalue++;
				remove_devpath_var(sih, &pvar);
				err = flash_update_nvbuf(osh, nvram_buf, nvramlen, board_buf, pvar, pvalue);
				if (err != BCME_OK)
					break;
			}
		}
		else
			break;
	}

	return err;
}
#endif  /* NVRAM_FLASH || BCMNVRAMR */

#if defined(NVRAM_FLASH) || defined(BCMNVRAMR)
static char*
get_default_nvram_filename(void)
{
	char *filename = NULL;

#if defined(BCMNVRAMR)
	if (nvram_path[0])
		filename = nvram_path;
	else
#endif /* BCMNVRAMR */
#if (defined(OEM_ANDROID) && defined(STB_SOC_WIFI))
		filename = NVRAM_FILE_OEM_ANDROID;
#else
		filename = NVRAM_FILE_DEFAULT;
#endif /* OEM_ANDROID && STB_SOC_WIFI */
	return filename;
}

static char*
get_board_nvram_filename(void)
{
	char *filename = NULL;

#if defined(BCMNVRAMR)
	if (board_nvram_path[0])
		filename = board_nvram_path;
#endif /* BCMNVRAMR */
	return filename;
}
#endif /* NVRAM_FLASH || BCMNVRAMR */

static int
nvram_file_init(void* sih)
{
	char *base = NULL, *nvp = NULL, *flvars = NULL;
	int err = 0, nvlen = 0;
	char *nvram_file;
	char *bbase = NULL, *bnvp = NULL;
	int bnvlen = 0;
	char *bnv_tmpbuf = NULL;

	/* freed in same function */
	base = nvp = MALLOC_NOPERSIST(si_osh((si_t *)sih), MAXSZ_NVRAM_VARS);
	if (base == NULL)
		return BCME_NOMEM;

	nvram_file = get_default_nvram_filename();
	if (nvram_file) {
		/* Init nvram from nvram file if they exist */
		err = initvars_file(sih, si_osh((si_t *)sih), nvram_file,  &nvp, (int*)&nvlen);
	} else
		err = BCME_ERROR;

	if (err == 0) {
		/* load board nvram file if board_nvram_path=filename specified */
		nvram_file = get_board_nvram_filename();
		if (nvram_file) {
			bbase = bnvp = MALLOC_NOPERSIST(si_osh((si_t *)sih), MAXSZ_NVRAM_VARS);
			if (bnvp == NULL) {
				err = BCME_NOMEM;
				goto exit;
			}
			/* Init board nvram from board nvram file if they exist */
			err = initvars_file(sih, si_osh((si_t *)sih), nvram_file,  &bnvp, (int*)&bnvlen);
			if (err == 0) {
				if (bnvlen) {
					bnv_tmpbuf = MALLOC_NOPERSIST(si_osh((si_t *)sih), bnvlen);
					if (bnv_tmpbuf == NULL) {
						err = BCME_NOMEM;
						goto exit;
					}
					memcpy(bnv_tmpbuf, bbase, bnvlen);
					nvp = base;
					nvlen--; /* initvars_file added one for the null character*/
					err = board_nvram_update(sih, si_osh((si_t *)sih), nvp, &nvlen, bnv_tmpbuf, bbase, bnvlen);
					nvp += nvlen;
					*nvp++ = '\0';
					nvlen = nvlen+1; /* add one for the null character */
					if (bnv_tmpbuf)
						MFREE(si_osh((si_t *)sih), bnv_tmpbuf, bnvlen);
					if (err != 0) {
						NVR_MSG(("No valid board NVRAM file present!!!\n"));
						err = 0;
					}
				}
			}
		}
	}
#ifdef NVRAM_FLASH
	if (err == 0) {
		nvp = base;
		nvlen--; /* initvars_file added one for the null character*/
		read_wlan_flash(sih, si_osh((si_t *)sih), &nvp, (int*)&nvlen);
	}
#endif /* NVRAM_FLASH */
	if (err != 0) {
		NVR_MSG(("No valid NVRAM file present!!!\n"));
		goto exit;
	}
	if (nvlen) {
		/* nvram data freed in si_detach */
		flvars = MALLOC_NOPERSIST(si_osh((si_t *)sih), nvlen);
		if (flvars == NULL)
			goto exit;
	}
	else
		goto exit;

	bcopy(base, flvars, nvlen);
	err = nvram_append(sih, flvars, nvlen);

exit:
	MFREE(si_osh((si_t *)sih), base, MAXSZ_NVRAM_VARS);
	if (bbase) {
		MFREE(si_osh((si_t *)sih), bbase, MAXSZ_NVRAM_VARS);
	}

	return err;
}

/** NVRAM file read for pcie NIC's */
static int
initvars_file(si_t *sih, osl_t *osh, char *nvram_file, char **nvramp, int *nvraml)
{
#if defined(BCMDRIVER)
	/* Init nvram from nvram file if they exist */
	char *nvram_buf = *nvramp;
	void	*nvram_fp = NULL;
	int ret = 0, len = 0;
	char *v = NULL;
	int32 sromrev = 0;

	nvram_fp = (void*)osl_os_open_image(nvram_file);
	if (nvram_fp != NULL) {
		if (!(len = osl_os_get_image_block(nvram_buf, MAXSZ_NVRAM_VARS, nvram_fp))) {
			NVR_MSG(("Could not read %s file\n", nvram_file));
			ret = BCME_ERROR;
			goto exit;
		}
	}
	else {
		NVR_MSG(("Could not open %s file\n", nvram_file));
		ret = BCME_ERROR;
		goto exit;
	}

	/* process nvram vars if user specified a text file instead of binary */
	len = process_nvram_vars(nvram_buf, len);
	if (sih) {
		v = findvar(nvram_buf, nvram_buf+len, SROMREV_STR);
		if (v) {
			sromrev = bcm_atoi(v);
		}
		switch(CHIPID(sih->chip)) {
		case BCM7271_CHIP_ID:
			if (sromrev < 11) {
				NVR_MSG(("%s do not have valid sromrev\n", nvram_file));
				ret = BCME_ERROR;
			}
			break;
		default:
			break;
		}
	}

	nvram_buf += len;
	*nvram_buf++ = '\0';
	*nvramp = nvram_buf;
	*nvraml = len+1; /* add one for the null character */

exit:
	if (nvram_fp)
		osl_os_close_image(nvram_fp);

	return ret;
#else /* BCMDRIVER */
	return BCME_ERROR
#endif /* BCMDRIVER */
}

int
nvram_file_read(char **nvramp, int *nvraml)
{
	int err = 0;
	char *nvram_file;

	nvram_file = get_default_nvram_filename();
	if (nvram_file) {
		/* Init nvram from nvram file if they exist */
		err = initvars_file(NULL, NULL, nvram_file, nvramp, nvraml);
	} else {
		NVR_MSG(("No valid NVRAM file present!!!\n"));
		err = BCME_ERROR;
	}
	return err;
}

int
BCMATTACHFN(nvram_append)(void *si, char *varlst, uint varsz)
{
	uint bufsz = VARS_T_OH;
	vars_t *new;

	/* nvram data freed in si_detach */
	if ((new = MALLOC_NOPERSIST(si_osh((si_t *)si), bufsz)) == NULL)
		return BCME_NOMEM;

	new->vars = varlst;
	new->bufsz = bufsz;
	new->size = varsz;
	new->next = vars;
	sortvars((si_t *)si, new);

	vars = new;

	return BCME_OK;
}

void
nvram_get_global_vars(char **varlst, uint *varsz)
{
	*varlst = vars->vars;
	*varsz = vars->size;
}

void
BCMATTACHFN(nvram_exit)(void *si)
{
	vars_t *this, *next;
	si_t *sih;

	sih = (si_t *)si;
	this = vars;

	if (this)
		MFREE(si_osh(sih), this->vars, this->size);

	while (this) {
		next = this->next;
		MFREE(si_osh(sih), this, this->bufsz);
		this = next;
	}
	vars = NULL;
}

static char *
findvar(char *vars_arg, char *lim, const char *name)
{
	char *s;
	int len;

	len = strlen(name);

	for (s = vars_arg; (s < lim) && *s;) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}

	return NULL;
}

#ifdef BCMSPACE
char *defvars = "il0macaddr=00:11:22:33:44:55\0"
		"boardtype=0xffff\0"
		"boardrev=0x10\0"
		"boardflags=8\0"
		"aa0=3\0"
		"sromrev=2";
#define	DEFVARSLEN	89	/* Length of *defvars */
#else /* !BCMSPACE */
char *defvars = "";
#endif	/* BCMSPACE */

static char *
nvram_get_internal(const char *name)
{
	vars_t *cur;
	char *v = NULL;
	const int len = strlen(name);

	for (cur = vars; cur; cur = cur->next) {
		/*
		 * The variables are sorted by length (everything
		 * NUM_VSIZES or longer is put in the last pool).  So
		 * we can resterict the sort to just those variables
		 * that match the length.  This is a surprisingly big
		 * speedup as there are many lookups during init.
		 */
		if (len >= NUM_VSIZES) {
			/* Scan all strings with len > NUM_VSIZES */
			v = findvar(cur->vars + cur->vsizes[NUM_VSIZES - 1],
			            cur->vars + cur->size, name);
		} else {
			/* Scan just the strings that match the len */
			v = findvar(cur->vars + cur->vsizes[len - 1],
			            cur->vars + cur->vsizes[len], name);
		}

		if (v) {
			return v;
		}
	}

#ifdef BCMSPACE
	v = findvar(defvars, defvars + DEFVARSLEN, name);
	if (v)
		NVR_MSG(("%s: variable %s defaulted to %s\n",
		         __FUNCTION__, name, v));
#endif	/* BCMSPACE */

	return v;
}

char *
nvram_get(const char *name)
{
	NVRAM_RECLAIM_CHECK(name);
	return nvram_get_internal(name);
}

int
BCMATTACHFN(nvram_set)(const char *name, const char *value)
{
	return 0;
}

int
BCMATTACHFN(nvram_unset)(const char *name)
{
	return 0;
}

int
BCMATTACHFN(nvram_reset)(void *si)
{
	return 0;
}

int
BCMATTACHFN(nvram_commit)(void)
{
	return 0;
}

static int
nvram_getall_internal(char *buf, int count)
{
	int len, resid = count;
	vars_t *this;

	this = vars;
	while (this) {
		char *from, *lim, *to;
		int acc;

		from = this->vars;
		lim = (char *)((uintptr)this->vars + this->size);
		to = buf;
		acc = 0;
		while ((from < lim) && (*from)) {
			len = strlen(from) + 1;
			if (resid < (acc + len))
				return BCME_BUFTOOSHORT;
			bcopy(from, to, len);
			acc += len;
			from += len;
			to += len;
		}

		resid -= acc;
		buf += acc;
		this = this->next;
	}
	if (resid < 1)
		return BCME_BUFTOOSHORT;
	*buf = '\0';
	return 0;
}

int
nvram_getall(char *buf, int count)
{
	NVRAM_RECLAIM_CHECK("nvram_getall");
	return nvram_getall_internal(buf, count);
}

#ifdef BCMQT
extern void nvram_printall(void);
/* QT: print nvram w/o a big buffer - helps w/memory consumption evaluation of USB bootloader */
void
nvram_printall(void)
{
	vars_t *this;

	this = vars;
	while (this) {
		char *from, *lim;

		from = this->vars;
		lim = (char *)((uintptr)this->vars + this->size);
		while ((from < lim) && (*from)) {
			printf("%s\n", from);
			from += strlen(from) + 1;
		}
		this = this->next;
	}
}
#endif /* BCMQT */

#if defined(NVRAM_FLASH) || defined(BCMNVRAMR)
static int
flash_update_nvbuf(osl_t *osh, char *nvram_buf, int *nvramlen, char *flshdev_data, char *pvar, char *pvalue)
{
	char *pnvvalue = NULL;
	char *pch = NULL;
	int n = 0;
	int32 value_each_subband = 0, panum = 0;
	bool ispavar = FALSE;
	uint16 *povalues = NULL;
	int err = BCME_OK;

	if (!nvram_buf || !flshdev_data || !nvramlen || !pvar || !pvalue)
		return BCME_ERROR;
	/*Only non-pavar and vaild pavar will return BCME_OK*/
	if ((err = pavar_check(osh, nvram_buf, flshdev_data, &value_each_subband, &panum, &ispavar, pvar, pvalue)) != BCME_OK)
		return BCME_ERROR;

	if ((pnvvalue = getvar(nvram_buf, pvar))) /*Modify values*/
	{
		if (ispavar){
			int idx = 0;
			int ponum = 0;
			ponum = panum/value_each_subband;
			if ((povalues = MALLOC_NOPERSIST(osh, (ponum * sizeof(uint16))))){
				n = 0;
				while ((pch = bcmstrtok(&pvalue, ",", NULL)) && (n < panum))
				{
					if((n % value_each_subband) == 0){
						povalues[idx] = (uint16)bcm_strtoul(pch, NULL, 16);
						idx++;
					}
					n++;
				}
				/*Only update power offset(A value) of pavars, use B, C, D values from nvram file*/
				update_po_value(osh, nvram_buf, pnvvalue, nvramlen, povalues, ponum, value_each_subband);
				MFREE(osh, povalues, (ponum * sizeof(uint16)));
			}
		}
		else{
			if (strcmp(pvar, SROMREV_STR)) /*sromrev var in flash is used to check pa version, so do not override the value from nvram file*/
				flash_modify_var(osh, nvram_buf, nvramlen, pvar, pvalue);
		}
	}
	else /*Add new var*/
		flash_add_new_var(osh, nvram_buf, nvramlen, pvar, pvalue);

	return err ;
}

static int
get_pavar_tbl(const pavars_t **pav, uint32 paparambwver, uint32 sromrev){
	int err = BCME_OK;

	if (pav == NULL)
		return BCME_ERROR;

	 /*Not support paparambwver, todo if we want to use paparambwver*/
	if (paparambwver != 0)
		*pav = pavars;
	else {
		/*srom revision number*/
		if (sromrev > 12)
			*pav = pavars_SROM13;
		if (sromrev == 12)
			*pav = pavars_SROM12;
		if (sromrev < 12)
			*pav = pavars;
	}
	return err;
}

/*
* if var is not pavar, return BCME_OK.
* if var is pavar and valid, return BCME_OK, else return BCME_ERROR.
*/
static int
pavar_check(osl_t *osh, char *nvram_buf, char *flshdev_data, int32 *value_each_subband, int *panum, bool *ispa, char *pvar, char *pvalue)
{
	int32 paparambwver = 0;
	int32 sromrev = 0, fsromrev = 0;
	const pavars_t *pav = pavars;
	int32 len = 0, n = 0;
	int err = BCME_ERROR;

	if(!osh || !nvram_buf || !flshdev_data || !value_each_subband || !panum || !ispa || !pvar || !pvalue )
		return BCME_ERROR;

	paparambwver = getintvar(nvram_buf, "paparambwver");
	sromrev = getintvar(nvram_buf, SROMREV_STR);
	fsromrev = getflashintvar(flshdev_data, SROMREV_STR);
	if((strlen(pvar) >= 4) && (!strncmp(pvar, PA5G_STR, strlen(PA5G_STR)) || !strncmp(pvar, PA2G_STR, strlen(PA2G_STR))))
		*ispa = TRUE;
	else
		*ispa = FALSE;

	if (*ispa) {
		if ((get_pavar_tbl(&pav, paparambwver, sromrev) == BCME_OK ) && (pav != pavars) && fsromrev && (sromrev == fsromrev)) {

			while (pav->phy_type != PHY_TYPE_NULL) {
				if (!strcmp(pvar, pav->vars)) {
					 /*only support sromrev >= 12, todo if we want to use sromrev < 12*/
					if (((pav->phy_type == PHY_TYPE_AC) || (pav->phy_type == PHY_TYPE_LCN20)) && (sromrev >= 12)) {
						if ((pav->bandrange == WL_CHAN_FREQ_RANGE_2G) ||
						   (pav->bandrange == WL_CHAN_FREQ_RANGE_2G_40)){
							*panum = 4;
							*value_each_subband = 4;
						}
						else if ((pav->bandrange ==
							WL_CHAN_FREQ_RANGE_5G_5BAND) ||
							(pav->bandrange ==
							WL_CHAN_FREQ_RANGE_5G_5BAND_40) ||
							(pav->bandrange ==
							WL_CHAN_FREQ_RANGE_5G_5BAND_80)){
							*panum = 20;
							*value_each_subband = 4;
						}
						else {
							*panum = 0;
							*value_each_subband = 0;
						}
					}
					break;
				}
				pav++;
			}
			if ((len = strlen(pvalue)) && (*panum)) {
				char* pabase = NULL;
				char* patmp = NULL;
				len++;
				patmp = pabase = (char*) MALLOC_NOPERSIST(osh, len);
				memcpy(patmp, pvalue, len);
				while ((bcmstrtok(&patmp, ",", NULL)))
					n++;

				if ((n == *panum) && (*panum >= *value_each_subband) && (*panum % *value_each_subband == 0)) /*check paparam*/
					err = BCME_OK;

				if (pabase)
					MFREE(osh, pabase, len);
			}
		}
	}
	else
		err = BCME_OK;

	return err;
}

static int update_po_value(osl_t *osh, char *nvram_buf, char *pnvvalue, int *nvramlen, uint16 *podata, int32 ponum, int32 value_each_subband)
{
	char *q = NULL;
	char *update = NULL;
	int param_idx = 0, poidx = 0;
	int next_po_value_idx = 0;
	int n = 0;
	int len_diff = 0;
	char *tmpbuf = NULL;
	int nvidx = 0 ;

	if (!osh || !pnvvalue || !nvramlen || !podata || !(tmpbuf =  MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS)))
		return BCME_ERROR;

	nvidx = pnvvalue - nvram_buf;
	while (ponum){
		update = pnvvalue;
		memset(tmpbuf, 0, MAXSZ_NVRAM_VARS);
		while(*(pnvvalue+n) != '\0'){ /*find "A" value */
			q = (pnvvalue+n);
			if (*q == ','){
				if (param_idx == next_po_value_idx){
					next_po_value_idx += value_each_subband;
					break;
				}
				else {
					update = q+1;
				}
				param_idx++;
			}
			n++;
		}
		len_diff = strlen("0xffff") - (q-update);
		memcpy(tmpbuf,q,(*nvramlen - nvidx - n));
		sprintf(update,"0x%04x",*(podata + poidx));
		update += (2*sizeof(uint16))+2; /*length of hex ascii fromat (0xffff)*/
		memcpy(update,tmpbuf,(*nvramlen - nvidx - n));
		*nvramlen += len_diff;
		poidx++;
		ponum--;
	}

	if (tmpbuf)
		MFREE(osh, tmpbuf, MAXSZ_NVRAM_VARS);

	return BCME_OK;
}

static int
flash_modify_var(osl_t *osh, char *nvram_buf, int *nvramlen, char *pvar, char *pvalue)
{
	char *tmpbuf =	NULL;
	int lendif = 0, tmplen = 0, n = 0, pidx = 0, valuelen = 0;
	char *pnvvalue = NULL;
	pnvvalue = getvar(nvram_buf, pvar);

	if (!osh || !nvram_buf || !nvramlen || !pvar || !pvalue || !(tmpbuf =  MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS)))
		return BCME_ERROR;

	lendif = strlen(pvalue) - strlen(pnvvalue);
	pidx = pnvvalue - nvram_buf;
	valuelen = (strlen(pnvvalue)+1);
	tmplen = *nvramlen-pidx - valuelen;
	memcpy(tmpbuf, pnvvalue + valuelen, tmplen);
	n += sprintf(pnvvalue+n, "%s", pvalue);
	*(pnvvalue+n) = '\0';
	memcpy((pnvvalue+n+1), tmpbuf, tmplen);
	*nvramlen += lendif;

	if (tmpbuf)
		MFREE(osh, tmpbuf, MAXSZ_NVRAM_VARS);

	return BCME_OK;
}

static int
flash_add_new_var(osl_t *osh, char *nvram_buf, int *nvramlen, char *pvar, char *pvalue)
{
	char *tmpbuf =	NULL;
	char *pnvvalue = NULL;
	int lendif = 0, tmplen = 0, n = 0;

	if (!osh || !nvram_buf || !nvramlen || !pvar || !pvalue)
		return BCME_ERROR;

	if (!(tmpbuf =  MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS)))
		return BCME_ERROR;

	pnvvalue = nvram_buf;
	tmplen = *nvramlen;
	memcpy(tmpbuf, pnvvalue, *nvramlen);
	lendif = strlen(pvar) + 1 + strlen(pvalue) + 1; /*var=value\0*/
	n += sprintf(pnvvalue, "%s=", pvar);
	n += sprintf(pnvvalue+n, "%s", pvalue);
	*(pnvvalue+n) = '\0';
	memcpy((pnvvalue+n+1), tmpbuf, tmplen);
	*nvramlen += lendif;

	if (tmpbuf)
		MFREE(osh, tmpbuf, MAXSZ_NVRAM_VARS);

	return BCME_OK;
}

/* Island write setting for NVRAM format is,  output of 'wl devpath' concatenate with pa params.
* Remove devpath to get nvram var.
*/
static int
remove_devpath_var(si_t *sih, char **pvar)
{
	int err = BCME_OK;
	int slen = 0, tokens = 0;
	uint32 num = 0, num2 = 0;
	char devpath[SI_DEVPATH_BUFSZ] = {'\0'};

	if ( !sih || !pvar)
		return BCME_ERROR;

	switch (BUSTYPE(sih->bustype)) {
		case SI_BUS:
		case JTAG_BUS:
			/* Island get  devpath by "wl devpath", and the path is sb/1/.
			* But when wl driver at nvram_init,  si_coreidx is 0 so that devpath get from si_devpath is sb/0/
			* So, it is not able to get the same devpath as Island get by si_devpath.
			* Use below method to check and to  remove devpath.
			*/
			tokens = sscanf(*pvar, DEVPATH_JTAG_BUS_FORMAT, &num);
			if (tokens == 1)
				slen = snprintf(devpath, SI_DEVPATH_BUFSZ, DEVPATH_JTAG_BUS_FORMAT, num);
			break;
		case PCI_BUS:
			tokens = sscanf(*pvar, DEVPATH_PCI_BUS_FORMAT, &num, &num2);
			if (tokens == 2)
				slen = snprintf(devpath, sizeof(devpath), DEVPATH_PCI_BUS_FORMAT, num, num2);
			break;
		case PCMCIA_BUS:
			tokens = sscanf(*pvar, DEVPATH_PCMCIA_BUS_FORMAT, &num, &num2);
			if (tokens == 2)
				slen = snprintf(devpath, SI_DEVPATH_BUFSZ,DEVPATH_PCMCIA_BUS_FORMAT, num, num2);
			break;
		default:
			break;
	}

	if (slen == 0) {
		tokens = 0;
		num = 0;
		tokens = sscanf(*pvar, "%d:", &num);
		if (tokens == 1) {
			memset(devpath, '\0', SI_DEVPATH_BUFSZ);
			slen = snprintf(devpath, sizeof(devpath), "%d:", num);
		}
	}

	if ((slen > 0) && (!memcmp(*pvar, devpath, strlen(devpath))) && (strlen(*pvar) > slen))
		*pvar += slen; /* Remove devpath */

	return err;
}
#endif /* NVRAM_FLASH || BCMNVRAMR */

#ifdef NVRAM_FLASH
/* returns the CRC8 of the nvram */
static uint8
soc_nvram_calc_crc(struct nvram_header *nvh)
{
	struct nvram_header tmp;
	uint8 crc;

	if (!nvh)
		return 0;

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init = htol32((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));
	crc = hndcrc8((uint8 *) &tmp + NVRAM_CRC_START_POSITION, 1, CRC8_INIT_VALUE);
	/* Continue CRC8 over data bytes */
	crc = hndcrc8((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);
	return crc;
}

static void
read_wlan_flash(si_t *sih, osl_t *osh, char **nvramp, int *nvraml)
{
	struct nvram_header *nvh = NULL;
	char *flshdev_buf = NULL;
	char *flshdev_tmpbuf = NULL;
	void *flshdev_fp = NULL;
	int flshdevlen = 0;
	char flshdevpath[32];
	char *ptmpdata = NULL;
	char *pflshdevdata = NULL;
	int maxdlen = 0;
	char *nvram_buf = NULL;
	int len = 0;

	if(!osh || ! nvramp || !nvraml)
		return;

	nvram_buf = *nvramp;
	len = *nvraml;
	memset(flshdevpath,'\0',sizeof(flshdevpath));
	if (find_wlanflash_dev(osh, flshdevpath,sizeof(flshdevpath)) == BCME_OK
		&& (flshdev_fp = (void*)osl_os_open_image(flshdevpath))
		&& (flshdev_buf = MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS))
		&& (flshdev_tmpbuf = MALLOC_NOPERSIST(osh, MAXSZ_NVRAM_VARS))
		&& ((flshdevlen = osl_os_get_image_block(flshdev_buf, MAXSZ_NVRAM_VARS, flshdev_fp)) > 0))
	{
		nvh = (struct nvram_header*)flshdev_buf;
		if ((nvh->magic == NVRAM_MAGIC) && (nvh->len >= sizeof(struct nvram_header)) && (soc_nvram_calc_crc(nvh) == (uint8)nvh->crc_ver_init))/*Signature*/
		{
			memcpy(flshdev_tmpbuf, flshdev_buf, MAXSZ_NVRAM_VARS);
			pflshdevdata = FLASHPDATA(flshdev_buf);
			ptmpdata = FLASHPDATA(flshdev_tmpbuf);
			maxdlen = (nvh->len) - (ptmpdata - flshdev_tmpbuf);
			board_nvram_update(sih, osh, nvram_buf, &len, ptmpdata, pflshdevdata, maxdlen);
		}
		else
			NVR_MSG(("Invalid nvram data in wlan flash\n"));
	}
	else
		NVR_MSG(("%s does not exist\n", WIFI_FLASH_PARTITION));

	nvram_buf += len;
	*nvram_buf++ = '\0';
	*nvramp = nvram_buf;
	*nvraml = len+1; /* add one for the null character */

	if (flshdev_fp)
		osl_os_close_image(flshdev_fp);
	if (flshdev_buf)
		MFREE(osh, flshdev_buf, MAXSZ_NVRAM_VARS);
	if (flshdev_tmpbuf)
		MFREE(osh, flshdev_tmpbuf, MAXSZ_NVRAM_VARS);
}
#endif /* NVRAM_FLASH */
