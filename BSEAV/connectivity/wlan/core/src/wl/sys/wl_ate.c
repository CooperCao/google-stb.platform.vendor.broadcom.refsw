/*
 * Broadcom 802.11abg Networking Device Driver
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



#include "wl_rte.h"
#include "wl_ate.h"
#include <wlc_phy_hal.h>
#include <sbchipc.h>
#include "wlc_bmac.h"
#include "wlc_hw_priv.h"
#include <rte.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <sbgci.h>
#include <rte_gpio.h>
#include <hndpmu.h>

#include <stdio.h>
#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */
/* Macros */
#define ATE_CMD_STR_LEN_MAX	100
#define WLC_ATE_CMD_PARAMS_MAX 20
#define ATE_CMDS_NUM_MAX 768
#define C_ULP_NBIT 9
#define ATE_CMD_CHAR_LEN 11

/* External references referred in this file */
extern hnd_dev_t *dev_list;
extern void hnd_wait_irq(si_t *sih);

/* Locally used variables */
static const char rstr_ate_cmd_def_chan[] = "ate_def_chan";
static const char rstr_ate_cmd_gpio_ip[] = "ate_gpio_ip";
static const char rstr_ate_cmd_gpio_op[] = "ate_gpio_op";
static uint32 ate_buffer_regval_size = 0;
ate_buffer_regval_t ate_buffer_regval[MAX_RSDB_MAC_NUM];

/* ATE global data structure */
ate_params_t ate_params;
char ate_buffer_sc[ATE_SAMPLE_COLLECT_BUFFER_SIZE] __attribute__ ((aligned (32)));
uint32 ate_buffer_sc_size = 0;

void wl_ate_set_buffer_regval(uint8 type, uint32 value, int core, uint sliceidx, uint chip);

/* Locally used ATE functions */
static int wl_ate_cmd_preproc(char **argv, char *cmd_str);
static bool wlc_ate_capable_chip(uint32 devid);
static void wl_ate_sc_init(wl_info_t *wl, hnd_dev_t *dev);
static void wl_ate_gpio_isr(uint32 stat, void* cbdata);
static void wl_ate_gpio_init(wlc_info_t *wlc, si_t *sih, hnd_dev_t *dev);
static void wl_ate_cmd_wait_sr(void);


void
wl_ate_set_buffer_regval(uint8 type, uint32 value, int core, uint sliceidx, uint chip)
{
	BCM_REFERENCE(core);
	/* WA - To make ATE REGVAL buffer backword compatible for ATE for 4349/4355/4359 chip */
	if ((CHIPID(chip) != BCM4364_CHIP_ID)) {
		if ((sliceidx == 1) && (core == 0)) {
			sliceidx = 0;
			core = 1;
		}
	}

	switch (type) {
	case GMULT_LPF:
		ate_buffer_regval[sliceidx].gmult_lpf = value;
		break;
	case GMULT_ADC:
		ate_buffer_regval[sliceidx].gmult_adc = value;
		break;
	case RCCAL_DACBUF:
		ate_buffer_regval[sliceidx].rccal_dacbuf = value;
		break;
	case CURR_RADIO_TEMP:
		ate_buffer_regval[sliceidx].curr_radio_temp = value;
		break;
	case RCAL_VALUE:
		ate_buffer_regval[sliceidx].rcal_value[core] = value;
		break;
	default:
		ASSERT(0);
		break;
	}
}

static void
wl_ate_sc_init(wl_info_t *wl, hnd_dev_t *dev)
{
	/* Initialize the sample buffer */
	bzero(ate_buffer_sc, ATE_SAMPLE_COLLECT_BUFFER_SIZE);
	printf("ATE: Sample Collect Buffer Init Done\n");
}

static void
wl_ate_gpio_isr(uint32 stat, void* cbdata)
{
	ate_params.cmd_proceed = TRUE;
}

static void
BCMATTACHFN(wl_ate_gpio_init)(wlc_info_t *wlc, si_t *sih, hnd_dev_t *dev)
{
	/* Configure the GPIOs IN */
	void *info_arg = NULL;
	if (getvar(wlc->pub->vars, rstr_ate_cmd_gpio_ip))
		ate_params.gpio_input = (uint8)getintvar(NULL, rstr_ate_cmd_gpio_ip);
	else {
		printf("ATE: Please provide ate_gpio_ip=0 in nvram\n");
		return;
	}


	if (getvar(wlc->pub->vars, rstr_ate_cmd_gpio_op))
		ate_params.gpio_output = (uint8)getintvar(NULL, rstr_ate_cmd_gpio_op);
	else {
		printf("ATE: Please provide ate_gpio_op=1 in nvram\n");
		return;
	}

	if (!AOB_ENAB(sih)) {
		/* Disable the PHY GPIOs */
		wlc_ate_gpiosel_disable(WLC_PI(wlc));
	}

	switch (dev->devid) {
		case BCM43012_D11N_ID:
			/* Configure the GPIO IN */
			/* Take over gpio control from cc */
			si_gpiocontrol(sih, (1 << ate_params.gpio_input),
			(1 << ate_params.gpio_input), GPIO_DRV_PRIORITY);

			/* GCI related GPIO programming */
			si_gci_chipcontrol(sih, 0, ~0, GPIO_SEL_0 | GPIO_SEL_1);
			si_gci_chipcontrol(sih, 1, ~0, GPIO_SEL_8 | GPIO_SEL_9);

			/* Register the GPIO interrupt handler (FALSE = edge-detect). */
			rte_gpio_handler_register((1 << ate_params.gpio_input), FALSE,
			wl_ate_gpio_isr, info_arg);

			/* make polarity opposite of the current value */
			si_gpiointpolarity(sih, (1 << ate_params.gpio_input),
				(si_gpioin(sih) & (1 << ate_params.gpio_input)), 0);


			/* Enable the GPIO INT specified */
			si_gpioeventintmask(sih, (1 << ate_params.gpio_input),
				(1 << ate_params.gpio_input), 0);

			/* Enable the GPIO */
			si_gpioouten(sih, 1 << ate_params.gpio_input,
				1 << ate_params.gpio_input, GPIO_HI_PRIORITY);

			printf("ATE: GPIO I/P %d init done devid: 0x%x\n",
				ate_params.gpio_input, dev->devid);

			/* Configure the GPIO OUT */
			si_gci_chipcontrol(sih, 0, ~0, GPIO_SEL_0 | GPIO_SEL_1);
			si_gci_chipcontrol(sih, 1, ~0, GPIO_SEL_8 | GPIO_SEL_9);

			/* Take over gpio control from cc and enable it for output */
			si_gpiocontrol(sih, ((1 << ate_params.gpio_output) |
				(1 << ate_params.gpio_input)), 0, GPIO_DRV_PRIORITY);
			si_gpioouten(sih, 1 << ate_params.gpio_output,
				1 << ate_params.gpio_output, GPIO_HI_PRIORITY);

#if ATE_DEBUG
			/* Usage of GPIO output */
			/* Writing to the GPIO now */
			si_gpioout(sih, 1 << ate_params.gpio_output,
				1 << ate_params.gpio_output, GPIO_HI_PRIORITY);
#endif /* ATE_DEBUG */

			printf("ATE: GPIO O/P %d init done devid: 0x%x\n",
				ate_params.gpio_output, dev->devid);
		break;

		case BCM4347_D11AC_ID:
		case BCM4361_D11AC_ID:
			{
				/* Configure the GPIO IN */
				uint8 cur_status, wake_mask = 1 << GCI_GPIO_STS_NEG_EDGE_BIT;

				si_gci_set_functionsel(sih, ate_params.gpio_input,
						CC4345_FNSEL_GCI0);
				si_enable_gpio_wake(sih, &wake_mask, &cur_status,
						ate_params.gpio_input, 0, 0);

				/* Enable wake on GciWake */
				si_gci_indirect(sih, 0, GCI_OFFSETOF(sih, gci_wakemask),
						(GCI_INTSTATUS_GPIOWAKE | GCI_INTSTATUS_GPIOINT),
						(GCI_INTSTATUS_GPIOWAKE | GCI_INTSTATUS_GPIOINT));

				if (hnd_enable_gci_gpioint(ate_params.gpio_input, wake_mask,
						wl_ate_gpio_isr, info_arg) == NULL) {
					printf("%s: Cannot register input gpio devid: 0x%x\n",
							__FUNCTION__, dev->devid);
				}

				printf("ATE: GPIO I/P %d init done cur_state: 0x%x devid: 0x%x\n",
						ate_params.gpio_input, cur_status, dev->devid);

				/* Configure the GPIO OUT */
				si_gci_enable_gpio(sih, ate_params.gpio_output,
					1 << ate_params.gpio_output, 0x00);

				printf("ATE: GPIO O/P %d init done devid: 0x%x\n",
					ate_params.gpio_output, dev->devid);
				break;
			}
		default:
			printf("ATE: Unsupported devid: 0x%x\n", dev->devid);
			break;

	}

	return;
}

static int
wl_ate_cmd_preproc(char **argv, char *cmd_str)
{
	int param_count;
	char *array_ptr = cmd_str;

	ASSERT(strlen(cmd_str) <= ATE_CMD_STR_LEN_MAX);

	for (param_count = 0; param_count < WLC_ATE_CMD_PARAMS_MAX; param_count++)
		argv[param_count] = 0;

	for (param_count = 0; *array_ptr != '\0'; param_count++) {
		argv[param_count] = array_ptr;
		for (; (*array_ptr != '\0') && (*array_ptr != ' '); array_ptr++);
			if (*array_ptr == '\0')
				break;
			*array_ptr = '\0';
			array_ptr++;
	}

	return param_count + 1;
}

static bool
wlc_ate_capable_chip(uint32 devid)
{
	bool retval = FALSE;
	switch (devid) {
		case BCM4345_D11AC2G_ID:
		case BCM4345_D11AC5G_ID:
		case BCM4345_D11AC_ID:
		case BCM43455_D11AC2G_ID:
		case BCM43455_D11AC5G_ID:
		case BCM43455_D11AC_ID:
		case BCM4349_D11AC_ID:
		case BCM4347_D11AC_ID:
		case BCM4361_D11AC_ID:
		case BCM43430_D11N2G_ID:
		case BCM43012_D11N_ID:
			retval = TRUE;
			break;
		default:
			retval = FALSE;
	}
	return retval;
}

void
wl_ate_cmd_proc(void)
{
	char *ate_str = NULL;
	char *argv[WLC_ATE_CMD_PARAMS_MAX];
	uint8 argc = 0;
	char ate_cmd_str[ATE_CMD_CHAR_LEN] = "ate_cmd";
	uint8 ate_cmd_str_len = strlen(ate_cmd_str);
	char ate_cmd_num[4];
	wl_info_t *wl = NULL;
	wlc_info_t *wlc = NULL;
	int delay = 0;

	if ((ate_params.ate_cmd_done == TRUE) ||	/* All commands executed */
		(ate_params.cmd_proceed == FALSE))		/* Waiting for GPIO trigger */
		return;

	ASSERT(ate_params.wl);
	wl = ate_params.wl;
	wlc = wl->wlc;

	if (ate_params.cmd_idx == 0) {
		printf("\nATE CMD : START!!!\n");
		/* Be prepared for a Wait for INT ATE command */
		ate_params.cmd_proceed = FALSE;
	}

	do {
		/* Preparing search string 'atecmdXX', to extract ATE commands from NVRAM */
		(void)snprintf(ate_cmd_num, sizeof(ate_cmd_num), "%03X", ate_params.cmd_idx);
		memcpy(&ate_cmd_str[ate_cmd_str_len], ate_cmd_num, sizeof(ate_cmd_str));
		ate_cmd_str[ate_cmd_str_len+3] = '\0';
		ate_str = getvar(wlc->pub->vars, ate_cmd_str);

		if (ate_str) {
			printf("ATE CMD%03X: %s\n", ate_params.cmd_idx, ate_str);
			argc = wl_ate_cmd_preproc(argv, ate_str);

			if (strcmp(argv[0], "ate_cmd_wait_gpio_rising_edge") == 0) {
				/* Execute the ATE command */
				if (ate_params.cmd_proceed == TRUE) {
					ate_params.cmd_proceed = FALSE;
					/* Proceed with the next ATE command */
					ate_params.cmd_idx++;
					printf("\n");
					continue;
				} else {
					return;
				}
			} else if (strcmp(argv[0], "ate_cmd_wait_sr") == 0) {
				/* Wait for sr cycle to occur
				 * clear the rentention bit(that indicates a SR sleep cycle)
				 * by writing 1 to it.
				 */
				wl_ate_cmd_wait_sr();

				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				printf("\n");
				continue;
			} else if (strcmp(argv[0], "ate_cmd_write_gpio") == 0) {
				/* Write to the assigned GPIO */
				si_gpioout(ate_params.sih, 1 << ate_params.gpio_output,
					(strcmp(argv[1], "0")) << ate_params.gpio_output,
					GPIO_HI_PRIORITY);

				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else if (strcmp(argv[0], "ate_cmd_delay_in_us") == 0) {
				delay = atoi(argv[1]);
				OSL_DELAY(delay);
				/* Proceed with the next ATE command */
				ate_params.cmd_idx++;
				continue;
			} else {
				if (argc > 1) {
					/* Execute the ATE wl command */
					if (!strcmp(argv[1], "channel"))
						wl_intrsoff(wl);

					do_wl_cmd((uint32)wlc->wl, argc, argv);

					if (!strcmp(argv[1], "channel") && (wlc->pub->up))
						wl_intrson(wl);
				} else {
					printf("ATE Command: Invalid command : %s. "
						"Num of params : %d\n", ate_str, argc);
				}
				ate_params.cmd_idx++;
			}
		} else {
			ate_params.cmd_idx++;
		}
	} while (ate_params.cmd_idx < ATE_CMDS_NUM_MAX);

	printf("ATE CMD : END!!!\n");

	/* All ATE commands done, update the variables accordingly */
	ate_params.cmd_idx = 0;
	ate_params.ate_cmd_done = TRUE;

	return;
}

void
wl_ate_init(si_t *sih, hnd_dev_t *bcmwl)
{
	hnd_dev_t *dev = bcmwl;
	wl_info_t *wl = NULL;
	wlc_info_t *wlc = NULL;

	/* Validate chip - ATE commands supported for some chips only */
	while (dev) {
		if (wlc_ate_capable_chip(dev->devid))
			break;
		dev = dev->next;
	}
	if (!dev) {
		printf("ATE: This chip is NOT supported for ATE operations!!!\n");
		ASSERT(FALSE);
		return;
	}
	wl = ate_params.wl = dev->softc;
	wlc = wl->wlc;
	ate_params.sih = sih;

	/* Init ATE params */
	ate_params.cmd_proceed = TRUE;
	ate_params.ate_cmd_done = FALSE;
	ate_params.cmd_idx = 0;
	ate_params.gpio_input = 0xFF;
	ate_params.gpio_output = 0xFF;

	/* Init the GPIOs, if needed */
	wl_ate_gpio_init(wlc, sih, dev);

	/* Configure the default channel */
	if (getvar(wlc->pub->vars, rstr_ate_cmd_def_chan)) {
		wlc->default_bss->chanspec =
			CH20MHZ_CHSPEC((uint8)getintvar(NULL, rstr_ate_cmd_def_chan));
	}

	/* Init the sample collect related params */
	wl_ate_sc_init(wl, dev);

	/* Initialize the Regval buffer related params */
	bzero((char *) &ate_buffer_regval, sizeof(ate_buffer_regval));
	ate_buffer_regval_size = sizeof(ate_buffer_regval);
	/* Done with init */
	printf("ATE: Init done\n");
}

static void
wl_ate_cmd_wait_sr(void)
{
	wlc_hw_info_t *wlc_hw = NULL;
	wlc_hw_info_t *other_wlc_hw = NULL;
	si_t *sih = NULL;
	d11regs_t *regs;
	int sicoreunit = 0;
	uint curr_core, dummy,	sdio_int, curr_int;
	uint32 req_bits, mac_ctl, other_mac_ctl;
	uint16 hostflag, other_hostflag;
	wlc_hw = ate_params.wlc_hw;
	sih = ate_params.sih;
#ifdef WLRSDB
	if (CHIPID(wlc->pub->sih->chip) == BCM4364_CHIP_ID) {
		wlc_info_t *other_wlc;
		other_wlc = wlc_rsdb_get_other_wlc(wlc_hw->wlc);
		other_wlc_hw = other_wlc->hw;
	}
#endif /* WLRSDB */
	sicoreunit = si_coreunit(wlc_hw->sih);
	printf("ATE: wait for sr command.....\n");
	printf("Disable SDIO ");
	OSL_DELAY(100);
	/* si switch disables interrupts so preserve masks? */
	si_switch_core(sih, SDIOD_CORE_ID, &curr_core, &curr_int);
	si_core_reset(sih, 0x0, 0x0);
	si_switch_core(sih, curr_core, &dummy, &sdio_int);
	si_restore_core(sih, curr_core, curr_int);
	OSL_DELAY(100);

	/* printf("Clr RCTL_MEM_RET_SLEEP\n"); */
	if (CHIPID(wlc->pub->sih->chip) != BCM4364_CHIP_ID) {
		si_corereg(sih,
			SI_CC_IDX,
			OFFSETOF(chipcregs_t, retention_ctl),
			RCTL_MEM_RET_SLEEP_LOG_MASK, RCTL_MEM_RET_SLEEP_LOG_MASK);

		regs = si_d11_switch_addrbase(wlc_hw->sih, MAC_CORE_UNIT_1);
		OR_REG(wlc_hw->osh, &regs->clk_ctl_st, CCS_FORCEHWREQOFF);
		si_d11_switch_addrbase(wlc_hw->sih, sicoreunit);
		req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL;
		AND_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, ~req_bits);
		mac_ctl = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
		W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol,
				(mac_ctl & ~MCTL_WAKE));

		/* Force deep sleep */
		printf("ATE: force deep sleep: Set ULP_NBIT\n");
		hostflag = wlc_bmac_read_shm(wlc_hw, MHF1);
		wlc_bmac_write_shm(wlc_hw, MHF1,
				hostflag | (1 << C_ULP_NBIT));
	} else {
		req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL;
		AND_REG(wlc_hw->osh, &wlc_hw->regs->clk_ctl_st, ~req_bits);
		mac_ctl = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
		W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol,
				(mac_ctl & ~MCTL_WAKE));

		/* Force deep sleep */
		printf("ATE: force deep sleep for slice 0\n");
		hostflag = wlc_bmac_read_shm(wlc_hw, MHF1);
		wlc_bmac_write_shm(wlc_hw, MHF1,
				hostflag | (1 << C_ULP_NBIT));

		req_bits = CCS_ERSRC_REQ_D11PLL | CCS_ERSRC_REQ_PHYPLL;
		AND_REG(other_wlc_hw->osh, &other_wlc_hw->regs->clk_ctl_st,
			~req_bits);
		other_mac_ctl = R_REG(other_wlc_hw->osh,
			&other_wlc_hw->regs->maccontrol);
		W_REG(other_wlc_hw->osh, &other_wlc_hw->regs->maccontrol,
				(other_mac_ctl & ~MCTL_WAKE));

		/* Force deep sleep */
		printf("ATE: force deep sleep for slice 1\n");
		other_hostflag =
			wlc_bmac_read_shm(other_wlc_hw, M_HOST_FLAGS1);
		wlc_bmac_write_shm(other_wlc_hw, M_HOST_FLAGS1,
				other_hostflag | (1 << C_ULP_NBIT));
	}

	printf("Going to wait for irq: wfi state\n");
	hnd_pmu_clr_int_sts_req_active(wlc_hw->osh, sih);
	hnd_wait_irq(sih);
	/* __asm__ __volatile__("wfi"); */

	printf("\nAWAKE .....\n");
	OSL_DELAY(100);
	si_switch_core(sih, SDIOD_CORE_ID, &curr_core, &curr_int);
	si_core_reset(sih, 0x0, 0x0);
	si_clear_backplane_to(sih);
	si_switch_core(sih, curr_core, &dummy, &sdio_int);
	si_restore_core(sih, curr_core, curr_int);
	OSL_DELAY(100);

	if (CHIPID(wlc->pub->sih->chip) != BCM4364_CHIP_ID) {
		regs = si_d11_switch_addrbase(wlc_hw->sih, MAC_CORE_UNIT_1);
		AND_REG(wlc_hw->osh, &regs->clk_ctl_st, ~CCS_FORCEHWREQOFF);

		si_d11_switch_addrbase(wlc_hw->sih, sicoreunit);
		mac_ctl = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);

		/* printf("Clr ULP_NBIT\n"); */
		wlc_bmac_write_shm(wlc_hw, MHF1,
				hostflag & ~(1 << C_ULP_NBIT));

		W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol,
				(mac_ctl | MCTL_WAKE));
	} else {
		mac_ctl = R_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol);
		printf("ATE: wakeup from sleep for slice 0\n");
		wlc_bmac_write_shm(wlc_hw, MHF1,
				hostflag & ~(1 << C_ULP_NBIT));
		W_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol,
				(mac_ctl | MCTL_WAKE));

		other_mac_ctl = R_REG(other_wlc_hw->osh,
			&other_wlc_hw->regs->maccontrol);
		printf("ATE: wakeup from sleep for slice 1\n");
		wlc_bmac_write_shm(other_wlc_hw, MHF1,
				other_hostflag & ~(1 << C_ULP_NBIT));
		W_REG(other_wlc_hw->osh, &other_wlc_hw->regs->maccontrol,
				(other_mac_ctl | MCTL_WAKE));
	}
	printf("Done, Executing wake sequence\n");
}
