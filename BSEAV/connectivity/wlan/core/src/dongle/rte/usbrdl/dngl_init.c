/*
 * USB remote download bootcode
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
#include <rdl_dbg.h>

si_t * _c_main(void);

/* Remote Download device struct */
extern hnd_dev_t bcmrdl;
#ifdef USB_HUB
extern hnd_dev_t usbhub_dev;
extern void hub_bind(void *hub_drv, void *usb_drv);
extern void usb_bind(void *rdl_drv, void *hub_drv);
#endif /* USB_HUB */

extern void sf_bootloader_check(void);
extern bool sflash_present_check_and_enable(void);
extern bool validate_sdr_image(struct trx_header *hdr);

void *jump_addr = NULL;

#define FLOPS_START_MASK		0xFF

#ifdef HSIC_SDR_DEBUG
/* defined in startarm-cr4.S */
extern volatile uint __chipsdrenable;
extern volatile uint __nopubkeyinotp;
extern volatile uint __imagenotdigsigned;
extern volatile uint __pubkeyunavail;
extern volatile uint __rsaimageverify;
#define HSIC_SDR_DEBUG_1(x)		__chipsdrenable = (x)
#define HSIC_SDR_DEBUG_2(x)		__nopubkeyinotp = (x)
#define HSIC_SDR_DEBUG_3(x)		__imagenotdigsigned = (x)
#define HSIC_SDR_DEBUG_4(x)		__pubkeyunavail = (x)
#define HSIC_SDR_DEBUG_5(x)		__rsaimageverify = (x)
#else
#define HSIC_SDR_DEBUG_1(x)
#define HSIC_SDR_DEBUG_2(x)
#define HSIC_SDR_DEBUG_3(x)
#define HSIC_SDR_DEBUG_4(x)
#define HSIC_SDR_DEBUG_5(x)
#endif /* HSIC_SDR_DEBUG */

#ifdef USB_HUB
static void usb_hub_bind(hnd_dev_t *bcmrdl, hnd_dev_t *usbhub_dev)
{
	usb_bind(bcmrdl->softc, usbhub_dev->softc);
	hub_bind(usbhub_dev->softc, bcmrdl->softc);
}
#endif /* USB_HUB */

static bool
load_validate_usb_image(si_t *sih)
{
	bool ret = TRUE;
#ifdef USB_HUB
	if (hnd_add_device(sih, &usbhub_dev, HUB_CORE_ID, BCM47XX_USBHUB_ID) != 0) {
		err(" add hub failed");
		ASSERT(0);
	}
#endif /* USB_HUB */

	/* Add the usb rdl device */
	if ((hnd_add_device(sih, &bcmrdl, USB_CORE_ID, BCM47XX_USBD_ID) == 0) ||
		(hnd_add_device(sih, &bcmrdl, USB11D_CORE_ID, BCM47XX_USBD_ID) == 0) ||
#ifdef USB_XDCI
		((bcmrdl.flags & (1 << RTEDEVFLAG_USB30)) &&
		(hnd_add_device(sih, &bcmrdl, USB30D_CORE_ID, BCM47XX_USB30D_ID) == 0)) ||
#endif
#ifdef USB_BDCI
		(hnd_add_device(sih, &bcmrdl, USB30D_CORE_ID, BCM47XX_USB30D_ID) == 0) ||
#endif
	    (hnd_add_device(sih, &bcmrdl, USB20D_CORE_ID, BCM47XX_USB20D_ID) == 0)) {

		dbg("Found a USB device\n");
	} else {
		err("Could not find a USB device\n");
		return FALSE;
	}

#ifdef USB_HUB
	usb_hub_bind(&bcmrdl, &usbhub_dev);
#endif /* USB_HUB */
	/* UP the rdl i/f */
	bcmrdl.ops->open(&bcmrdl);

	err("usbbldr:idling\n");

	return ret;
}

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
	hdr = (struct trx_header *)(SI_FLASH2);
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
chkpcie_dev(si_t *sih)
{
	/* For now if USB hostif is not active,
	 * it means PCIE.
	 */
	return CHIP_HOSTIF_PCIE(sih);
}

static bool
chk_usb_dev(si_t *sih)
{
	return CHIP_HOSTIF_USB(sih);
}

static bool
chk_sflash_dev(si_t *sih)
{
#ifndef SFLASH_BOOT
	if (CHIPID(sih->chip) == BCM4360_CHIP_ID)
		return ((sih->chipst & CST4360_SFLASH) == CST4360_SFLASH);
	if (CHIPID(sih->chip) == BCM4335_CHIP_ID)
		return sflash_present_check_and_enable();
	if (CHIPID(sih->chip) == BCM4345_CHIP_ID)
		return sflash_present_check_and_enable();
#endif /* !SFLASH_BOOT */

	return FALSE;
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
		HSIC_SDR_DEBUG_1(0);
		return TRUE;
	}

	printf("SDR enabled \n");
	pkb = (unsigned char *)getvar(NULL, "pubkey");
	if (!pkb) {
		printf("Public key not configured, skipping validation\n");
		HSIC_SDR_DEBUG_2(0);
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
		HSIC_SDR_DEBUG_3(0);
		return FALSE;
	}

	pkbsz = strlen((char *)pkb);
	if (pkbsz/2 < (RSA_KEY_SIZE/8)) {
		printf("Failed. \nPublic key not available\n");
		HSIC_SDR_DEBUG_4(0);
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
		HSIC_SDR_DEBUG_5(1);
		printf(" Pass\n");
	}
	else {
		HSIC_SDR_DEBUG_5(0);
		printf(" Fail\nrsa err:%d stat:%d\n", err, stat);
	}
	return (((err == 0) && (stat)) ? TRUE:FALSE);
}

/* The SDR image format in TCM after Download/Copy (From PCIE/USB/Sflash)
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
 * In case of PCIE, Host will download the image to TCM in this format.
 *
 * CASE-II:
 * In case of SFlash, bootloader will copy to TCM in this format.
 *
 * CASE-III:
 * In case of USB, bootloader will download and copy to TCM in this format.
 */


static bool
load_validate_pcie_image(si_t *sih)
{
	bool ret = TRUE;
	struct trx_header *hdr = (struct trx_header *)(_rambottom
				 - sizeof(struct trx_header) - 4);

	/* Check if we have valid TRX header at top of RAM.
	 * In PCIE hostif case, host is expected to place
	 * TRX header for the image at top of RAM.
	 */
	printf("PCIE host interface active... ");
	if (hdr->magic == TRX_MAGIC) {
		printf("found image in RAM\n");

		/* Validate the signature */
		if (!validate_sdr_image(hdr))
			ret = FALSE;
		else
			jump_addr = (void *)hdr->offsets[TRX_OFFSETS_JUMPTO_IDX];
	} else {
		printf("no image in RAM\n");
		ret = FALSE;
	}

	if (ret) {
		printf("Starting at %p\n", jump_addr);
		hnd_cpu_jumpto(jump_addr);
	}

	return ret;
}

static void
sflash_load_fail(si_t *sih)
{

	/* we want to proceed with USB download, when USB
	 * hostif is active
	 */
	if (CHIP_HOSTIF_USB(sih))
		return;

	printf("Disabling armcore\n");
	/* In PCIE hostif case, Hold CR4 in wait, no valid
	 * image in Sflash. Host will decide to re-enable
	 * for retry download.
	 */
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
			{chkpcie_dev, load_validate_pcie_image, NULL},
			{chk_sflash_dev, load_validate_sflash_image, sflash_load_fail},
			{chk_usb_dev, load_validate_usb_image, NULL},
	         };	
#endif /* BCM_SDRBL */


#ifdef BCMQT
extern void nvram_printall(void);
#endif

#ifdef USB_XDCI
static bool  usbd_is30(si_t *sih);

static bool  usbd_is30(si_t *sih)
{
	bool usb30d = FALSE;
	uint32 cs;

	cs = sih->chipst;

	/* need to check IFC since CST4350_USB30D_MODE is not reliable */
	if (CST4350_CHIPMODE_USB30D(cs) || CST4350_CHIPMODE_USB30D_WL(cs) ||
		CST4350_CHIPMODE_HSIC30D(cs)) {
		usb30d = TRUE;
	}
#ifdef BCM4350_FPGA
	usb30d = TRUE;
#endif

	return usb30d;
}
#endif /* USB_XDCI */


/* threadx application thread */
si_t * _c_main(void)
{
	char chn[8];
	si_t *sih;

#ifdef BCM_SDRBL
	struct loader_desc *ldev;
#endif /* BCM_SDRBL */

#ifndef SFLASH_BOOT
	sf_bootloader_check();
#endif /* !SFLASH_BOOT */
	/* Basic initialization */
	sih = hnd_init();

	/* Initialize and turn caches on */
	caches_on();

	printf("\n\nRTE (usbrdl) v%s running on BCM%s r%d @ %d/%d/%d MHz.\n",
	       EPI_VERSION_STR, bcm_chipname(sih->chip, chn, 8), sih->chiprev,
	       si_alp_clock(sih) / 1000000, si_clock(sih) / 1000000,
	       si_cpu_clock(sih) / 1000000);
	/* Add the usb rdl device */
#ifdef USB_XDCI
	if (usbd_is30(sih))
		bcmrdl.flags = (1 << RTEDEVFLAG_USB30);
#endif
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
	load_validate_usb_image(sih);
#endif /* BCM_SDRBL */

	return (sih);
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

#ifdef BCM_SDRBL
bool sflash_present_check_and_enable(void)
{
	volatile uint32 spromctrl, otpstatus, cc_clkdiv;
	volatile uint16 otp_sflash_bits, otp_sflash_type, otp_sflash_clkdiv;

	spromctrl = si_cc_get_reg32(CC_SROM_CTRL);
	/* Check for OTP present & select bits */
	if ((spromctrl & SPROM4335_OTP_SELECT) && (spromctrl & SPROM4335_OTP_PRESENT)) {
		otpstatus = si_cc_get_reg32(CC_OTPST);
		if (otpstatus & OTPS_READY) {
			/* In OTP following test bits are used for SFLASH. */
			/* Bit 320 => SFLASH present */
			/* Bit 321 => SFLASH type */
			/* Bit 322 - Bit 325  => clkdiv derived internally from
			* backplane for SFLASH
			*/
			otp_sflash_bits = si_cc_get_reg16(CC_SROM_OTP + CC4335_SROM_OTP_SFLASH);
			if (otp_sflash_bits & CC4335_SROM_OTP_SFLASH_PRESENT) {
				otp_sflash_type = (otp_sflash_bits &
					CC4335_SROM_OTP_SFLASH_TYPE) ? 1 : 0;

				otp_sflash_clkdiv = (otp_sflash_bits &
					CC4335_SROM_OTP_SFLASH_CLKDIV_MASK) >>
					CC4335_SROM_OTP_SFLASH_CLKDIV_SHIFT;

				/* Configure sdio pads for SFLASH mode */
				si_gci_preinit_upd_indirect(CC_GCI_CC_OFFSET_2,
					CC4335_GCI_FUNC_SEL_PAD_SDIO,
					CC4335_GCI_FUNC_SEL_PAD_SDIO);

				/* Clear the sflash type bit. */
				si_gci_preinit_upd_indirect(CC_GCI_CC_OFFSET_5,
					(((uint32)otp_sflash_type) <<
					CC4335_GCI_STRAP_OVERRIDE_SFLASH_TYPE),
					(1 << CC4335_GCI_STRAP_OVERRIDE_SFLASH_TYPE));

				/* Program SFLASH clkdiv if programmed. */
				if (otp_sflash_clkdiv) {
					cc_clkdiv = si_cc_get_reg32(CC_CLKDIV);
					/* Clear the bits 24 to 28 */
					cc_clkdiv &= ~(CC4335_SFLASH_CLKDIV_MASK);
					/* Set bits (25 - 28) taking from OTP. */
					cc_clkdiv |= (otp_sflash_clkdiv <<
						CC4335_SFLASH_CLKDIV_SHIFT);
					si_cc_set_reg32(CC_CLKDIV, cc_clkdiv);
				}

				/* clear the sflash present bit. */
				si_gci_preinit_upd_indirect(CC_GCI_CC_OFFSET_5,
					CC4335_GCI_STRAP_OVERRIDE_SFLASH_PRESENT,
					CC4335_GCI_STRAP_OVERRIDE_SFLASH_PRESENT);

				return TRUE;
			}
		}
	}
	return FALSE;
}
#endif /* BCM_SDRBL */

/*
 * For 4360/4350 USB package, if there is a bootloader image in flash, just use it
 * regardless of SDR setting.
 */
void sf_bootloader_check(void)
{
	volatile uint32 chipid, chipstatus;
	struct trx_header *hdr;
	bool flashoption = FALSE;

	chipid = *((volatile uint32 *)(SI_ENUM_BASE(NULL) + CC_CHIPID));
	chipstatus = *(volatile uint32 *)(SI_ENUM_BASE(NULL) + CC_CHIPST);

	/* check for supported chips along with pkg/chipstat/mode options */
	switch ((chipid & CID_ID_MASK)) {
#ifdef BCM_SDRBL
		case BCM4360_CHIP_ID:
			if ((((chipid &  CID_PKG_MASK) >> CID_PKG_SHIFT) == 0x1) &&
				(chipstatus & CST4360_MODE_USB) &&
				(chipstatus & CST4360_SFLASH)) {
				/* flash option present */
				flashoption = TRUE;
			}
			break;
		case BCM4335_CHIP_ID:
		case BCM4345_CHIP_ID:
			flashoption = sflash_present_check_and_enable();
			break;
#endif /* BCM_SDRBL */
		case BCM4350_CHIP_ID:
		case BCM4354_CHIP_ID:
		case BCM43556_CHIP_ID:
		case BCM43558_CHIP_ID:
		case BCM43566_CHIP_ID:
		case BCM43568_CHIP_ID:
		case BCM43569_CHIP_ID:
		case BCM43570_CHIP_ID:
		case BCM4358_CHIP_ID:
			if ((CST4350_CHIPMODE_HSIC30D(chipstatus) ||
				CST4350_CHIPMODE_HSIC20D(chipstatus) ||
				CST4350_CHIPMODE_USB20D(chipstatus) ||
				CST4350_CHIPMODE_USB30D(chipstatus) ||
				CST4350_CHIPMODE_USB30D_WL(chipstatus)) &&
				(chipstatus & CST4350_SFLASH_PRESENT))
				/* flash option present */
				flashoption = TRUE;
			break;
		case BCM4373_CHIP_ID:
			if ((CST4373_CHIPMODE_USB20D(chipstatus)) &&
				(chipstatus & CST4373_SFLASH_PRESENT))
				/* flash option present */
				flashoption = TRUE;
			break;
			/* no chips matched. */
		default:
			break;
	}

	/* now check whether flash is populated */
	if (flashoption) {
		hdr = (struct trx_header *)(SI_FLASH2);

		if (hdr->magic != TRX_MAGIC) {
			return;
		}
		if (!(hdr->flag_version & TRX_BOOTLOADER)) {
			return;
		}
		load_validate_sflash_image(NULL);
	}
}
