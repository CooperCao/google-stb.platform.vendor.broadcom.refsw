/*
 * Initialization and support routines for self-booting compressed image.
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
#include <bcmdefs.h>
#include <osl.h>
#include <siutils.h>
#include <wlioctl.h>
#include <hndcpu.h>
#include <bcmdevs.h>
#include <epivers.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <dngl_dbg.h>
#include <dngl_bus.h>
#include <hndtcam.h>
#include <bcmsdpcm.h>
#ifdef MSGTRACE
#include <dngl_msgtrace.h>
#endif
#include <event_log.h>
#ifdef LOGTRACE
#include <dngl_logtrace.h>
#endif
#include <bcmpcie.h>
#include <rte_dev.h>
#include <rte_mem.h>
#include <rte_tcam.h>
#include <rte.h>

#include <hnd_boot.h>
#include <hnd_event.h>

/* entry C function from assembly */
static si_t *c_init(void);

extern hnd_dev_t bcmwl;	/* WL device struct */

extern hnd_dev_t sdpcmd_dev;
extern hnd_dev_t usbdev_dev;
extern hnd_dev_t pciedngl_dev;
extern hnd_dev_t pciedev_dev;
extern hnd_dev_t m2md_dev;

#ifdef WIFI_REFLECTOR
void wifi_reflector_init(void);
#endif

#if defined(BCMM2MDEV) && defined(BCMM2MDEV_ENABLED)
static const char BCMATTACHDATA(busname)[] = "M2M";
#elif defined(BCMPCIEDEV) && defined(BCMPCIEDEV_ENABLED)
static const char BCMATTACHDATA(busname)[] = "PCIE";
#else
static const char BCMATTACHDATA(busname)[] = "error";
#endif

#ifdef RNDIS
static const char BCMATTACHDATA(busproto)[] = "RNDIS";
#elif BCMMSGBUF
static const char BCMATTACHDATA(busproto)[] = "MSG_BUF";
#else
static const char BCMATTACHDATA(busproto)[] = "CDC";
#endif

static const char BCMATTACHDATA(rstr_nocrc)[] = "nocrc";
static const char BCMATTACHDATA(rstr_crcchk)[] = "crcchk";
static const char BCMATTACHDATA(rstr_crcadr)[] = "crcadr";
static const char BCMATTACHDATA(rstr_crclen)[] = "crclen";
static const char BCMATTACHDATA(rstr_POLL)[] = "-POLL";
static const char BCMATTACHDATA(rstr_empty)[] = "";
static const char BCMATTACHDATA(rstr_RECLAIM)[] = "-RECLAIM";


static void get_FWID(void);

/** Writes to end of data section, so not to last RAM word */
static void
BCMATTACHFN(get_FWID)(void)
{
	uint8 *tagsrc = (uint8 *)_fw_meta_data;
	uint8 *tagdst = (uint8 *)&gFWID;

	tagdst[0] = tagsrc[27];
	tagdst[1] = tagsrc[28];
	tagdst[2] = tagsrc[29];
	tagdst[3] = tagsrc[30];
}


#if defined(RTE_CRC32_BIN) && defined(BCMSDIODEV_ENABLED) && !defined(mips)

/* For locations that may differ from download */
struct modlocs {
	uint32 vars;
	uint32 varsz;
	uint32 memsize;
	uint32 rstvec;
	void   *armregs;
	uint32 watermark;
	uint32 chiptype;
	uint32 armwrap;
	void  *ram_regs;
	uint32 ram_rev;
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	uint32 rambottom;
	uint32 atcmrambase;
#endif
	uint32  stackbottom;
	uint32 fwid;
};

uint32   crc_computed = 0xdeadbeef;

static void
BCMATTACHFN(chk_image)(void)
{
	/* Default to entire image (excluding the padded crc and fwtag) */
	uint8 *crcadr = (uint8 *)hnd_get_rambase();
	uint32 crclen = (_end - (char *)crcadr);
	uint32 crcchk = ~(*(uint32 *)_fw_meta_data);
	uint32 crcval;

	/* Will need to save/restore modified locations */
	extern char *_vars;
	extern uint32 _varsz, _memsize, orig_rst;
	extern uint32 arm_wrap, chiptype, _stackbottom;
	extern void *arm_regs;
#if defined(__ARM_ARCH_7A__)
	extern uint32 sysmem_rev;
	extern void *sysmem_regs;
#else
	extern uint32 socram_rev;
	extern void *socram_regs;
#endif

	struct modlocs newvals;

	/*
	 * if (_varsz > NVRAM_ARRAY_SIZE) startarm.S has already overwritten
	 * part of memory, so there is not much we can do but fail miserably
	 */
	while (_varsz > NVRAM_ARRAY_MAXSIZE);

	/* Bail if nvram explicity suppresses CRC check */
	if (getintvar(_vars, rstr_nocrc) == 1)
		goto done;


	/* Save possibly-modified locations and reset to original values */
	newvals.vars = (uint32)_vars;
	newvals.varsz = _varsz;
	newvals.memsize = _memsize;
	newvals.armregs = arm_regs;
	newvals.watermark = __watermark;
	newvals.chiptype = chiptype;
	newvals.armwrap = arm_wrap;
#if defined(__ARM_ARCH_7A__)
	newvals.ram_regs = (void *)sysmem_regs;
	newvals.ram_rev = sysmem_rev;
#else
	newvals.ram_regs = (void *)socram_regs;
	newvals.ram_rev = socram_rev;
#endif

#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	newvals.rambottom = _rambottom;
	newvals.atcmrambase = _atcmrambase;
#else
	/* __ARM_ARCH_7M__ */
	/* BACKGROUND:- In CR4(7R) ROM starts with 0x000, */
	/* but in CM3(7M) RAM starts with 0x000 */
	newvals.rstvec = *(uint32*)0;
#endif
	newvals.stackbottom = _stackbottom;
	newvals.fwid = gFWID;

#if defined(BCMHOSTVARS)
	/* Temporarily copy NVRAM to Arena and clear the original NVRAM area for CRC purposes */
	(void) memcpy(_end + 4, _vars, _varsz);
	(void) memset(_vars, 0, _varsz);
#endif	

	__watermark = 0xbbadbadd;
	_vars = NULL;
	_varsz = _memsize = 0;
	arm_wrap = 0;
	arm_regs = NULL;
#if defined(__ARM_ARCH_7R__)	/* See startarm-cr4.S */
	chiptype = 1;
	socram_rev = -1;
	socram_regs = (void *)-1;
	_rambottom = _atcmrambase = 0xbbadbadd;
#elif defined(__ARM_ARCH_7A__)	/* See startarm-ca7.S */
	chiptype = 1;
	sysmem_rev = -1;
	sysmem_regs = (void *)-1;
	_rambottom = _atcmrambase = 0xbbadbadd;
#elif defined(__ARM_ARCH_7M__) /* See startarm-cm3.S */
	socram_rev = chiptype = 0;
	socram_regs = NULL;
#endif
	_stackbottom = 0;
	 gFWID = 0;


#ifdef BCMDBG_ARMRST
	*(uint32*)0 = orig_rst;
	orig_rst = 0;
#endif

	/* Generate the checksum */
	crcval = hndcrc32(crcadr, crclen, CRC32_INIT_VALUE);

	/* Restore modified locations */
	_vars = (char*)newvals.vars;
	_varsz = newvals.varsz;
	_memsize = newvals.memsize;
	orig_rst = *(uint32*)0;
	arm_regs = newvals.armregs;
	__watermark = newvals.watermark;
	chiptype = newvals.chiptype;
	arm_wrap = newvals.armwrap;
#if defined(__ARM_ARCH_7A__)
	sysmem_regs = newvals.ram_regs;
	sysmem_rev = newvals.ram_rev;
#else
	socram_regs = newvals.ram_regs;
	socram_rev = newvals.ram_rev;
#endif
#if defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7A__)
	_rambottom = newvals.rambottom;
	_atcmrambase = newvals.atcmrambase;
#else
	/* __ARM_ARCH_7M__ */
	/* BACKGROUND:- In CR4(7R) ROM starts with 0x000, */
	/* but in CM3(7M) RAM starts with 0x000 */
	*(uint32*)0 = newvals.rstvec;
#endif
	_stackbottom = newvals.stackbottom;
	gFWID = newvals.fwid;

#if defined(BCMHOSTVARS)
	/* Copy back NVRAM data to _vars, and clear Arena */
	(void) memcpy(_vars, _end + 4, _varsz);
	(void) memset(_end + 4, 0, _varsz);
#endif	

#if defined(SR_ATTACH_MOVE)
	{
		/* A subset of ATTACH text/data has been relocated to the save-restore memory
		 * region. There are now multiple ATTACH sections, which are no longer contiguous,
		 * and the memory in between them is the initial heap/stack. But by the time this
		 * function has been called, the stack has obviously been modified and is in use.
		 * The CRC from the start of RAM to the end of the normal text/data sections has
		 * been calculated above ('crc1' below). Calculate the remaining CRC in sections:
		 *
		 *    [ txt/dat/bss/rclm ] [future heap] [ ...stack... (active) ] [srm]
		 *    |<----- crc1 ------>|<---------- crc2 --------->|<--crc3-->|<crc4>
		 *                      _end                          |-128-| local var stackbottom
		 */

		/* Calculate CRC from the end of the normal text/data sections to the start of the
		 * stack pointer (with a bit extra for good measure).
		 */
		uint32 stack_bottom = 0xdeaddead;
		crcadr = (uint8 *)_end;
		crclen = ((uint8 *)&stack_bottom - crcadr - 128);
		crcval = hndcrc32(crcadr, crclen, crcval);

		/* Calculate CRC for stack - don't use the actual stack which has been modified.
		 * Just run the CRC on a block of zeros.
		 */
		crclen = (_srm_start - _end - crclen);
		crcval = hndcrc32(crcadr, crclen, crcval);

		/* Calculate CRC for ATTACH text/data moved to save-restore region. */
		crcadr = (uint8 *)_srm_start;
		crclen = (_srm_end - (char *)crcadr);
		crcval = hndcrc32(crcadr, crclen, crcval);
	}
#endif /* SR_ATTACH_MOVE */

	crc_computed = crcval;
	while (crcval != crcchk);

done:;

}

#endif /* RTE_CRC32_BIN && BCMSDIODEV_ENABLED && !mips */

#ifdef USB_XDCI
static bool  usbd_is30(si_t *sih);

static bool  usbd_is30(si_t *sih)
{
	bool usb30d = FALSE;
	uint32 cs;

	cs = sih->chipst;

	dbg("cc status %x", cs);

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


#define CLK_MPT(clk)		(((clk) + 50000) / 1000000)
#define CLK_KPT(clk)		((((clk) + 50000) - CLK_MPT(clk) * 1000000) / 100000)

static char BCMATTACHDATA(rstr_banner)[] =
	"\nRTE (%s-%s%s%s) %s on BCM%s r%d @ %d.%d/%d.%d/%d.%dMHz\n";

static si_t *
BCMATTACHFN(c_init)(void)
{
	chipcregs_t *cc;
	uint16 id;
	si_t *sih;
	osl_t *osh;
	char chn[8];
	hnd_dev_t *netdev = NULL;
	hnd_dev_t *busdev = NULL;


	/* Basic initialization */
	sih = hnd_init();

	/* clear the watchdog counter which may have been set by the bootrom download code */
	si_watchdog(sih, 0);

	osh = si_osh(sih);

#ifdef mips
	if (MFC0(C0_STATUS, 0) & ST0_NMI)
		err("NMI bit set, ErrorPC=0x%x", MFC0(C0_ERREPC, 0));
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
	/* TODO: Check flags in the arm's resetlog */
#endif

	if (((cc = si_setcoreidx(sih, SI_CC_IDX)) != NULL) &&
	    (R_REG(osh, &cc->intstatus) & CI_WDRESET)) {
		err("Watchdog reset bit set, clearing");
		W_REG(osh, &cc->intstatus, CI_WDRESET);
	}

	/* Initialize and turn caches on */
	caches_on();

#ifdef EVENT_LOG_COMPILE
	  /* BUS LOG */
	event_log_set_init(osh, EVENT_LOG_SET_BUS, EVENT_LOG_BUS_BLOCK_SIZE);
	event_log_set_expand(osh, EVENT_LOG_SET_BUS, EVENT_LOG_BUS_BLOCK_SIZE);
	event_log_set_destination_set(EVENT_LOG_SET_BUS, SET_DESTINATION_HOST);

	  /* ERROR LOG  for directing errors to proper log set buffers */
	event_log_set_init(osh, EVENT_LOG_SET_ERROR, EVENT_LOG_ERROR_BLOCK_SIZE);
	event_log_set_expand(osh, EVENT_LOG_SET_ERROR, EVENT_LOG_ERROR_BLOCK_SIZE);
	event_log_set_destination_set(EVENT_LOG_SET_ERROR, SET_DESTINATION_HOST);

	/* WL LOG */
	event_log_set_init(osh, EVENT_LOG_SET_WL, EVENT_LOG_WL_BLOCK_SIZE);
	event_log_set_expand(osh, EVENT_LOG_SET_WL, EVENT_LOG_WL_BLOCK_SIZE);
	event_log_set_destination_set(EVENT_LOG_SET_WL, SET_DESTINATION_HOST);

#ifdef PCI_TRACE_BY_DEFAULT
	event_log_tag_start(EVENT_LOG_TAG_PCI_TRACE, EVENT_LOG_SET_BUS,
		EVENT_LOG_TAG_FLAG_LOG);
#endif /* PCI_TRACE_BY_DEFAULT */
	event_log_tag_start(EVENT_LOG_TAG_PCI_ERROR, EVENT_LOG_SET_BUS,
		EVENT_LOG_TAG_FLAG_LOG);
	/* All errors are mapped to error log set */
	event_log_tag_start(EVENT_LOG_TAG_PCI_ERROR, EVENT_LOG_SET_ERROR,
		EVENT_LOG_TAG_FLAG_LOG|EVENT_LOG_TAG_FLAG_PRINT);

#if defined(HEALTH_CHECK)
	event_log_tag_start(EVENT_LOG_TAG_HEALTH_CHECK_ERROR, EVENT_LOG_SET_ERROR,
		EVENT_LOG_TAG_FLAG_LOG);
#endif

#ifdef LOGTRACE
	/* Initialize event log host sender */
	logtrace_init(osh);
#endif /* LOGTRACE */
#endif /* EVENT_LOG_COMPILE */

#ifdef MSGTRACE
	/* Initialize message log host sender */
	msgtrace_init(osh);
#if defined(BCMSDIODEV_ENABLED)
	/* Let the host known the address of msgtrace.
	 * If there is a crash before sending the first event, then the host can download
	 * msgtrace.
	 */
	sdpcm_shared.msgtrace_addr = (uint32)msgtrace_get_addr();
#endif
#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		pciedev_shared.msgtrace_addr = (uint32)msgtrace_get_addr();
	}
#endif
#endif /* MSGTRACE */

#ifdef USB_XDCI
	if (usbd_is30(sih))
		usbdev_dev.flags = (1 << RTEDEVFLAG_USB30);
#endif
	/* Print the banner */
	printf(rstr_banner,
	       busname, busproto,
#ifdef RTE_POLL
	       rstr_POLL,
#else
	       rstr_empty,
#endif
#ifdef BCM_RECLAIM_INIT_FN_DATA
	       rstr_RECLAIM,
#else
	       rstr_empty,
#endif
	       EPI_VERSION_STR,
	       bcm_chipname(si_chipid(sih), chn, sizeof(chn)), sih->chiprev,
	       CLK_MPT(si_alp_clock(sih)), CLK_KPT(si_alp_clock(sih)),
	       CLK_MPT(si_clock(sih)), CLK_KPT(si_clock(sih)),
	       CLK_MPT(si_cpu_clock(sih)), CLK_KPT(si_cpu_clock(sih)));

	/* Add the USB or SDIO/PCMCIA/PCIE device.  Only one may be defined
	 * at a time, except during a GENROMTBL build where both are
	 * defined to make sure all the symbols are pulled into ROM.
	 */
#if !defined(BCMPCIEDEV) && !defined(BCMM2MDEV)
#error "Bus type undefined"
#endif

#ifdef DONGLEOVERLAYS
	hnd_overlay_prep();
#endif



#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		trace("  c_init: add PCIE device\n");
		if (hnd_add_device(sih, &pciedev_dev, PCIE2_CORE_ID, 0x43ff) == 0) {
			busdev = &pciedev_dev;
			bus_ops = &pciedev_bus_ops;
			proto_ops = &msgbuf_proto_ops;
		}
	}
#endif /* BCMPCIEDEV */

#if defined(BCMM2MDEV) && defined(BCMM2MDEV_ENABLED)
	trace("  c_init: add M2M device\n");
	if (hnd_add_device(sih, &m2md_dev, M2MDMA_CORE_ID, 0x4999) == 0) {
		busdev = &m2md_dev;
		bus_ops = &m2md_bus_ops;
		proto_ops = &cdc_proto_ops;
	}
#endif /* BCMM2MDEV && BCMM2MDEV_ENABLED */

#ifndef BCM4350_FPGA
	/* Add the WL device, they are exclusive */
	if ((id = si_d11_devid(sih)) == 0xffff)
		id = BCM4318_D11G_ID;
	trace("add WL device 0x%x", id);
	if (hnd_add_d11device(sih, &bcmwl, id) == 0)
		netdev = &bcmwl;
#else
	id = 0;
	netdev = &bcmwl;
#endif /* BCM4350_FPGA */

	ASSERT(busdev);
	ASSERT(netdev);
	/* If USB/SDIO/PCMCIA/PCI device is there then open it */
	if (busdev != NULL && netdev != NULL) {
#ifndef BCM4350_FPGA
		if (bus_ops->binddev(busdev, netdev,
			si_numd11coreunits(sih)) < 0)
			err("%s%s device binddev failed", busname, busproto);
#endif

#ifndef BCMPCIEDEV_ENABLED
		if (busdev->ops->open(busdev))
			err("%s%s device open failed", busname, busproto);
#endif

	}

	/* Initialize hnd_event */
	hnd_event_init(si_osh(sih), busdev);

	return sih;
}

/*
 * Common code for triggering any iovar/ioctl's after firmware initialization.
 * Currently used by reflcetor and ULP
 */
#if defined(WIFI_REFLECTOR) || defined(BCMULP)
typedef struct _init_cmds_t {
	uint32	cmd;
	uint32	len;
	char	*data;
	int	value;
} init_cmds_t;

#define MAX_INITCMD_BUFLEN	(32)

static void
init_cmds_process(const init_cmds_t *wifi_init_cmds, int count)
{
	int i;
	char buf[MAX_INITCMD_BUFLEN];
	const char *src;
	int idx = 0;

	hnd_dev_t *bcmwl_data = &bcmwl;
	for (i = 0; i < count; ++i) {
		if ((strlen(wifi_init_cmds[i].data) + sizeof(int) + 1) > MAX_INITCMD_BUFLEN) {
			printf("%s: ignoring cmd idx[%d]\n", __FUNCTION__, i);
			continue;
		}
		src = (char*)&wifi_init_cmds[i].value;
		if (wifi_init_cmds[i].data != NULL) {
			idx = strlen(wifi_init_cmds[i].data);
			memcpy(buf, wifi_init_cmds[i].data, idx);
			buf[idx] = '\0';
		} else
			idx = 0;
		buf[++idx] = src[0];
		buf[++idx] = src[1];
		buf[++idx] = src[2];
		buf[++idx] = src[3];

		bcmwl_data->ops->ioctl(bcmwl_data, wifi_init_cmds[i].cmd,
			buf, wifi_init_cmds[i].len, NULL, NULL, FALSE);
	}
}
#endif /* defined(WIFI_REFLECTOR) || defined (BCMULP) */

#ifdef WIFI_REFLECTOR
void
wifi_reflector_init(void)
{
	static const init_cmds_t wifi_init_cmds[] = {
		{WLC_UP, 0x0, NULL, 0x0},
		{WLC_SET_VAR, 0x8, "mpc", 0},
		{WLC_SET_WSEC, 0x4, NULL, 0x0},
		{WLC_SET_VAR, 0xf, "slow_timer", 999999},
		{WLC_SET_VAR, 0xf, "fast_timer", 999999},
		{WLC_SET_VAR, 0x12, "glacial_timer", 999999},
		{WLC_LEGACY_LINK_BEHAVIOR, 0x04, NULL, 0x1},
		{WLC_SET_MONITOR, 0x4, NULL, 0x1}
	};
	init_cmds_process(wifi_init_cmds, ARRAYSIZE(wifi_init_cmds));
}
#endif /* WIFI_REFLECTOR */

#ifdef BCMULP
static void ulp_post_reclaim(void)
{
	static const init_cmds_t ulp_init_cmds[] = {
		{WLC_SET_VAR, 0xf, "ulp_wlc_up", 0x0}
	};
	init_cmds_process(ulp_init_cmds, ARRAYSIZE(ulp_init_cmds));
}
#endif /* WIFI_REFLECTOR */


/* chip specific defines to check for default slave decode errors */
#define BCM4355Bx_DEFAULT_SLAVE_WRAPPER_ADDR	0x1810d000
#define BCM4355Bx_DEFSLAVE_ERRLOG_ARM_AXIID	0x0b

#define BCM4364_DEFAULT_SLAVE_WRAPPER_ADDR	0x1810e000
#define BCM4364_DEFSLAVE_ERRLOG_ARM_AXIID	0x0b

#define BCM4345Cx_DEFAULT_SLAVE_WRAPPER_ADDR    0x1810b000
#define BCM4345Cx_DEFSLAVE_ERRLOG_ARM_AXIID     0x0b

static uint32
BCMATTACHFN(rte_check_iginore_defslave_decode_error)(void)
{
	uint32 def_slave_addr;
	aidmp_t *def_slave;
	uint8 axi_id;
	uint32 val;
	uint32 slave_err_addr = 0;

	/* check for the chip specific values addr, axiid for ARM */
	/* handle the axi error the default way */

#if defined(BCM4345)
        def_slave_addr = BCM4345Cx_DEFAULT_SLAVE_WRAPPER_ADDR;
        axi_id = BCM4345Cx_DEFSLAVE_ERRLOG_ARM_AXIID;
#elif defined(BCM4355)
	def_slave_addr = BCM4355Bx_DEFAULT_SLAVE_WRAPPER_ADDR;
	axi_id = BCM4355Bx_DEFSLAVE_ERRLOG_ARM_AXIID;
#elif defined(BCM4364)
	def_slave_addr = BCM4364_DEFAULT_SLAVE_WRAPPER_ADDR;
	axi_id = BCM4364_DEFSLAVE_ERRLOG_ARM_AXIID;
#else
	def_slave_addr = 0;
	axi_id = 0;
#endif /* BCM4345 */
	def_slave = (aidmp_t *)def_slave_addr;
	if (def_slave == NULL)
		return slave_err_addr;

	/* read errlog register */
	val = R_REG(NULL, &def_slave->errlogstatus);
	if ((val & AIELS_TIMEOUT_MASK) == AIELS_DECODE) {
		val = R_REG(NULL, &def_slave->errlogid);
		/* check that transaction is from the CR4 Prefetch unit */
		if (val & axi_id) {
			slave_err_addr = R_REG(NULL, &def_slave->errlogaddrlo);
			W_REG(NULL, &def_slave->errlogdone, AIELD_ERRDONE_MASK);
			while ((val = R_REG(NULL, &def_slave->errlogstatus)) &
					AIELS_TIMEOUT_MASK);
		}
	}
	return slave_err_addr;
}

void c_image_init(void);
static uint32 boot_slave_err_addr = 0;

void
BCMATTACHFN(c_image_init)(void)
{
#if defined(RTE_CRC32_BIN) && defined(BCMSDIODEV_ENABLED) && !defined(mips)
	chk_image();
#endif /* (RTE_CRC32_BIN) && defined(BCMSDIODEV_ENABLED) && !defined(mips) */
	get_FWID();

#if defined(BCMCHIPID) && (BCMCHIPID == BCM4355_CHIP_ID)
	{
		uint32 *wr_cap_addr =
			(uint32 *)OSL_UNCACHED(SI_NIC400_GPV_BASE + SI_GPV_WR_CAP_ADDR);
		W_REG(NULL, wr_cap_addr, SI_GPV_WR_CAP_EN);
	}
#endif /* defined(BCMCHIPID) && (BCMCHIPID == BCM4355_CHIP_ID) */

	/* CPU Instruction prefetch unit can generate accesses to undef memory before MPU init */
	boot_slave_err_addr = rte_check_iginore_defslave_decode_error();

}

/* c_main is non-reclaimable, as it is the one calling hnd_reclaim */
si_t *_c_main(void);

si_t *
_c_main(void)
{
	si_t *sih;

#ifdef BCMPCIEDEV_ENABLED
	hnd_dev_t *busdev = &pciedev_dev;
#endif

#ifdef BCMTCAM
	/* Load patch table early */
	hnd_tcam_load_default(hnd_get_rambase());
#endif

	/* Call reclaimable init function */
	sih = c_init();


#if defined(RSOCK)
#ifdef BCM_RECLAIM_INIT_FN_DATA
	bcm_reclaimed = TRUE;
#endif /* BCM_RECLAIM_INIT_FN_DATA */
#ifdef BCMTCAM
	hnd_tcam_reclaim();
#endif
	hnd_reclaim();
#endif 

#ifdef BCMPCIEDEV_ENABLED
	if (busdev->ops->open(busdev))
		err("%s%s device open failed", busname, busproto);
#endif

#ifdef WIFI_REFLECTOR
	/* Get the interface up */
	wifi_reflector_init();
#endif
#ifdef BCMULP
	/* Get the interface up */
	if (si_is_warmboot())
		ulp_post_reclaim();
#endif
	return sih;
}
