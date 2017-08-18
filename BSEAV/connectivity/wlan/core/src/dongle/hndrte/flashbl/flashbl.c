/*
 * flash bootloader
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
 * $Id: flashbl.c 428247 2013-10-08 03:37:15Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <sbmemc.h>
#include <sflash.h>
#include <siutils.h>
#include <bcmsrom.h>
#include <epivers.h>
#include <trxhdr.h>
#include <usbrdl.h>
#ifdef BCM_SDRBL
#include <bcmcrypto/bcmtomcrypt.h>
#endif /* BCM_SDRBL */
#include <rte_dev.h>
#include <rte.h>

void c_main(unsigned long ra);

static si_t *sih;

/* Remote Download device struct */
hnd_dev_t bcmrdl;

extern void sf_bootloader_check(void);
extern bool validate_sdr_image(struct trx_header *hdr);

void *jump_addr = NULL;
#define FLOPS_START_MASK		0xFF


static bool
load_validate_sflash_image(si_t *sih)
{
	bool ret = TRUE;
	struct trx_header *hdr;
	void * copy_address;

	/* Check if we have valid TRX header at bottom of flash.
	 * In SFLASH if available, trx header is expected at
	 * bottom of FLASH.
	 */
	printf("Flash present... ");
	hdr = (struct trx_header *)(SI_SFLASH);
	if (hdr->magic == TRX_MAGIC) {
#ifdef BCM_SDRBL
		char *ram_trx_base = (char *)(_rambottom
			- sizeof(struct trx_header) - 4);
#endif /* BCM_SDRBL */
		char *imgbase =  (char *)hdr;
		imgbase += SIZEOF_TRX(hdr);

		unsigned long imgsz = hdr->offsets[TRX_OFFSETS_DLFWLEN_IDX]
					 + hdr->offsets[TRX_OFFSETS_NVM_LEN_IDX];
#ifdef BCM_SDRBL
		imgsz += hdr->offsets[TRX_OFFSETS_DSG_LEN_IDX]
			+ hdr->offsets[TRX_OFFSETS_CFG_LEN_IDX];

		printf("found image in Flash\n");
		/* Top of RAM is actually used for bootloader stack.
		 * But early stage of bootloader would have created
		 * space enough for TRX header. So we can safely
		 * copy the TRX header to top of RAM.
		 */
		printf("Copying trx from:%p to:%p\n", hdr, ram_trx_base);
		memcpy(ram_trx_base, hdr, sizeof(struct trx_header));
#endif /* BCM_SDRBL */

		/* Load the image:[nvram]:signature:[cfg] to bottom
		 * of RAM.
		 */
#ifdef FLOPS_SUPPORT
		copy_address = (void *)(hdr->offsets[TRX_OFFSETS_JUMPTO_IDX] & (~FLOPS_START_MASK));
#else
		copy_address = (void *)(hdr->offsets[TRX_OFFSETS_JUMPTO_IDX] & (~1));
#endif
		printf("Copying image from:%p to:%p %lu bytes\n", imgbase, copy_address, imgsz);
#ifdef BCM_SDRBL
		if (((uint32)copy_address + imgsz) > _rambottom) {
			printf("invalid image\n");
			ret = FALSE;
		} else
#endif /* BCM_SDRBL */
			memcpy(copy_address, imgbase, imgsz);

#ifdef BCM_SDRBL
		/* Validate the signature for sflash FW */
		if (ret) {
			if (!(hdr->flag_version & TRX_BOOTLOADER)) {
				if (!validate_sdr_image(hdr))
					ret = FALSE;
			}
		}
#endif /* BCM_SDRBL */
		if (ret)
			jump_addr = (void *)hdr->offsets[TRX_OFFSETS_JUMPTO_IDX];
	} else {
		printf("no image in Flash\n");
		ret = FALSE;
	}

	if (ret) {
		printf("Jumping To New Image in Flash.\n");
		printf("Starting at %p\n", jump_addr);
		hnd_cpu_jumpto(jump_addr);
	}
	printf("validate failed\n");

	return ret;
}

#ifdef BCM_SDRBL
#define MAX_LOADER_DESC (sizeof(loader_dsc)/sizeof(loader_dsc[0]))


static bool
chk_sflash_dev(si_t *sih)
{
	return TRUE;
}

bool
validate_sdr_image(struct trx_header *hdr)
{
	unsigned char *ib;
	unsigned long isz;
	unsigned char pk[128];
	unsigned char *pkb;
	unsigned long pkbsz;
	unsigned char *sig;
	unsigned long ssz;
	int	stat = 0, i;
	int	err;

	/* No validation required if SDR is disabled */
	if (!CHIP_SDRENABLE(sih)) {
		return TRUE;
	}

	printf("SDR enabled \n");
	pkb = (unsigned char *)getvar(NULL, "pubkey");
	if (!pkb) {
		printf("Public key not configured, skipping validation\n");
		return TRUE;
	}

#ifdef FLOPS_SUPPORT
	ib = (unsigned char *)DL_BASE;
#else
	ib = (unsigned char *)(_atcmrambase + (hdr->offsets[TRX_OFFSETS_JUMPTO_IDX] & (~1)));
#endif
	isz = hdr->offsets[TRX_OFFSETS_DLFWLEN_IDX] + hdr->offsets[TRX_OFFSETS_NVM_LEN_IDX];
	sig = (unsigned char *)((unsigned long)(ib) + isz);
	ssz = hdr->offsets[TRX_OFFSETS_DSG_LEN_IDX];

	printf("Image size:%lu Sig :%p sigsz:%lu\n", isz, sig, ssz);
	printf("Validating image....");

	if (!ssz) {
		printf("Failed. \nImage is not digitally signed\n");
		return FALSE;
	}

	pkbsz = strlen((char *)pkb);
	if (pkbsz/2 < (RSA_KEY_SIZE/8)) {
		printf("Failed. \nPublic key not available\n");
		return FALSE;
	}

	/* Convert public key back to binary format */
	for (i = 0; i < sizeof(pk); i++) {
		unsigned char c[] = "xx";

		c[0] = pkb[i*2];
		c[1] = pkb[(i*2) + 1];
		pk[i] = (unsigned  char)bcm_strtoul((char *)c, NULL, 16);
	}


	err = rsa_image_verify_hash(ib, isz, pk, sizeof(pk), sig, ssz,
		LTC_PKCS_1_V1_5, "sha256", &stat);

	if ((err == 0) && stat) {
		printf(" Pass\n");
	}
	else {
		printf(" Fail\nrsa err:%d stat:%d\n", err, stat);
	}
	return (((err == 0) && (stat)) ? TRUE:FALSE);
}

/* The SDR image format in TCM after Download/Copy (From Sflash)
 * looks like:
 *
 *	TCM Bottom -> -----------------
 *		      |   FW Image    |
 *		      |               |
 *		      |               |
 *		      |               |
 *		      |               |
 *		      -----------------
 *		      |Optional Nvram |
 *		      -----------------
 *		      |Sig of FW & Nv |
 *		      -----------------
 *		      | Optional cfg  |
 *		      -----------------
 *		      |               |
 *		      |               |
 *		      |               |
 *		      |               |
 *		      -----------------
 *		      |  TRX hdr      |
 *	TCM Top ->    -----------------
 *
 * CASE-I:
 * In case of SFlash, bootloader will copy to TCM in this format.
 */


static void
sflash_load_fail(si_t *sih)
{
	printf("Disabling armcore\n");

	disable_arm_irq();
	while (1) {
		hnd_cpu_wait(sih);
	}
}

struct loader_desc {
	bool (* chkdev) (si_t *sih);
	bool (* loadimg_validate) (si_t *sih);
	void (* fail) (si_t *sih);
} loader_dsc[] = {
			{chk_sflash_dev, load_validate_sflash_image, sflash_load_fail},
	         };	
#endif /* BCM_SDRBL */


#ifdef BCMQT
extern void nvram_printall(void);
#endif

void
c_main(unsigned long ra)
{
	char chn[8];
#ifdef BCM_SDRBL
	struct loader_desc *ldev;
#endif /* BCM_SDRBL */

	BCMDBG_TRACE(0x77708);
#ifndef SFLASH_BOOT
	sf_bootloader_check();
#endif /* !SFLASH_BOOT */
	/* Basic initialization */
	sih = hnd_init();
	BCMDBG_TRACE(0x77709);

	/* Initialize and turn caches on */
	caches_on();

	printf("\n\nRTE (flashrdl) v%s running on BCM%s r%d @ %d/%d/%d MHz.\n",
	       EPI_VERSION_STR, bcm_chipname(sih->chip, chn, 8), sih->chiprev,
	       si_alp_clock(sih) / 1000000, si_clock(sih) / 1000000,
	       si_cpu_clock(sih) / 1000000);
#ifdef BCMQT
	printf("bootloader NVRAM:\n");
	nvram_printall();
#endif /* BCMQT */
#ifdef SFLASH_BOOT
	printf("flash bootloader\n");
#endif /* SFLASH_BOOT */
#ifdef BCM_SDRBL
	for (ldev = loader_dsc; ldev < &loader_dsc[MAX_LOADER_DESC]; ldev++) {

		/* Check and load, if required validate the image */
		if (!ldev->chkdev(sih) || !ldev->loadimg_validate(sih)) {
			if (ldev->fail)
				ldev->fail(sih);
			else
				continue;
		}
	}
#else /* BCM_SDRBL */
	while (1) {
		hnd_cpu_wait(sih);
	}
#endif /* BCM_SDRBL */
}

#ifdef ZLIB
/* Functions required by zlib */
void *calloc(int num, int size);
void free(void *ptr);

void *
calloc(int num, int size)
{
	void *ptr;

	ptr = hnd_malloc(size*num);
	if (ptr)
		bzero(ptr, size*num);

	return (ptr);
}

void
free(void *ptr)
{
	hnd_free(ptr);
}
#endif /* ZLIB */

/* At the end so the whole data segment gets copied */
int input_data = 0xdead;

void sf_bootloader_check(void)
{
	volatile uint32 chipid;
	struct trx_header *hdr;
	bool flashoption = FALSE;

	chipid = *((volatile uint32 *)(SI_ENUM_BASE(NULL) + CC_CHIPID));

	/* check for supported chips along with pkg/chipstat/mode options */
	switch ((chipid & CID_ID_MASK)) {
			/* no chips matched. */
		default:
			break;
	}

	/* now check whether flash is populated */
	if (flashoption) {
		hdr = (struct trx_header *)(SI_SFLASH);

		if (hdr->magic != TRX_MAGIC) {
			return;
		}
		if (!(hdr->flag_version & TRX_BOOTLOADER)) {
			return;
		}
		load_validate_sflash_image(NULL);
	}
}
