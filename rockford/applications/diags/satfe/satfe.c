/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "satfe.h"
#include "satfe_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bast_g3.h"

#ifdef SATFE_USE_BFSK
#ifdef BFSK_PROTOCOL_ECHO
#include "satfe_fsk.h"
#else
#include "satfe_ftm.h"
#endif
#endif

/* #define SATFE_USE_FLOAT */


/* global variables */
SATFE_Chip *g_pSATFE = NULL;
int g_SATFE_curr_chip_idx = 0;
bool ResetStatus=false;


/* local functions */
bool SATFE_Command_read(struct SATFE_Chip*, int, char**);
bool SATFE_Command_write(struct SATFE_Chip*, int, char**);
bool SATFE_Command_config(struct SATFE_Chip*, int, char**);
bool SATFE_Command_ta(struct SATFE_Chip*, int, char**);
bool SATFE_Command_abort(struct SATFE_Chip*, int, char**);
bool SATFE_Command_status(struct SATFE_Chip*, int, char**);
bool SATFE_Command_ver(struct SATFE_Chip*, int, char**);
bool SATFE_Command_help(struct SATFE_Chip*, int, char**);
bool SATFE_Command_diseqc_reset(struct SATFE_Chip*, int, char**);
bool SATFE_Command_vtop(struct SATFE_Chip*, int, char**);
bool SATFE_Command_vbot(struct SATFE_Chip*, int, char**);
bool SATFE_Command_tone(struct SATFE_Chip*, int, char**);
bool SATFE_Command_acw(struct SATFE_Chip*, int, char**);
bool SATFE_Command_send(struct SATFE_Chip*, int, char**);
bool SATFE_Command_monitor(struct SATFE_Chip*, int, char**);
bool SATFE_Command_ber(struct SATFE_Chip*, int, char**);
bool SATFE_Command_reset_status(struct SATFE_Chip*, int, char**);
bool SATFE_Command_chip(struct SATFE_Chip*, int, char**);
bool SATFE_Command_channel(struct SATFE_Chip*, int, char**);
bool SATFE_Command_symbol_rate(struct SATFE_Chip*, int, char**);
bool SATFE_Command_acq_ctl(struct SATFE_Chip*, int, char**);
bool SATFE_Command_carrier_offset(struct SATFE_Chip*, int, char**);
bool SATFE_Command_mode(struct SATFE_Chip*, int, char**);
bool SATFE_Command_repeat(struct SATFE_Chip*, int, char**);
bool SATFE_Command_test_acq(struct SATFE_Chip*, int, char**);
bool SATFE_Command_switch(struct SATFE_Chip*, int, char**);
bool SATFE_Command_monitor_lock_events(struct SATFE_Chip*, int, char**);
bool SATFE_Command_dft_test(struct SATFE_Chip*, int, char**);
bool SATFE_Command_plc_trk(struct SATFE_Chip*, int, char**);
bool SATFE_Command_plc_acq(struct SATFE_Chip*, int, char**);
bool SATFE_Command_test_status_indicators(struct SATFE_Chip*, int, char**);
bool SATFE_Command_search_range(struct SATFE_Chip*, int, char**);
bool SATFE_Command_tuner_filter(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_debug(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_blind_scan(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_blind_scan_psd(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_lock_transponders(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_xport(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_config_diseqc(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_network_spec(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_check_timing_lock(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_search_tone(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_power_down(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_power_up(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_test_power_down(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_test_send(SATFE_Chip *pChip, int argc, char **argv);
#ifndef SATFE_USE_BFSK
bool SATFE_Command_ftm_uc_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_read(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_uc_version(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_options(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_rxmask(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_hard_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_register(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_echo(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_ping(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_xtune(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_lock(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_unlock(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_test_register(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_test_xtune(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_test_echo(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_power_down(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_ftm_power_up(SATFE_Chip *pChip, int argc, char **argv);
#endif
bool SATFE_Command_bcm3445_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_bcm3445_map_output(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_bcm3445_config(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_bcm3445_read(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_bcm3445_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_vsense(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_config_gpio(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_get_gpio(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_set_gpio(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_freq_sweep(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_cwc(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_regdump(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_delay(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_mi2c_write(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_mi2c_read(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_reset_channel(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_soft_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_pilot(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_bert(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_amc_scrambling_seq(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_get_snr_range(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_adc(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_diseqc_voltage(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_diseqc_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_test_mpeg_count(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_test_lock_event(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_test_snr(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_dft_debug(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_Command_reboot(SATFE_Chip *pChip, int argc, char **argv);

#ifndef SATFE_USE_BFSK
BERR_Code SATFE_Ftm_ResetUc(SATFE_Chip *pChip);
void      SATFE_Ftm_LogWrite(SATFE_Chip *pChip, uint8_t *buf, uint8_t len);
void      SATFE_Ftm_LogRead(SATFE_Chip *pChip, uint8_t *buf, uint8_t len);
BERR_Code SATFE_Ftm_SendMessage(SATFE_Chip *pChip, uint8_t *pSendBuf, uint8_t sendLen, uint8_t *pRcvBuf, int timeoutMsec, uint8_t expReplyLen, uint8_t expReplyCmd);
BERR_Code SATFE_Ftm_GetMessage(SATFE_Chip *pChip, uint8_t *pBuf, uint8_t *pLen, int timeoutMsec);
BERR_Code SATFE_Ftm_Register(SATFE_Chip *pChip, uint8_t num_tuners);
#endif


SATFE_Command SATFE_CommandSet[] =
{
   {"read", SATFE_Command_read},
   {"r", SATFE_Command_read},
   {"write", SATFE_Command_write},
   {"w", SATFE_Command_write},
   {"config", SATFE_Command_config},
   {"ta", SATFE_Command_ta},
   {"status", SATFE_Command_status},
   {"s", SATFE_Command_status},
   {"abort", SATFE_Command_abort},
   {"ver", SATFE_Command_ver},
   {"help", SATFE_Command_help},
   {"diseqc_reset", SATFE_Command_diseqc_reset},
   {"reset_diseqc", SATFE_Command_diseqc_reset},
   {"vtop", SATFE_Command_vtop},
   {"vbot", SATFE_Command_vbot},
   {"tone", SATFE_Command_tone},
   {"acw", SATFE_Command_acw},
   {"send", SATFE_Command_send},
   {"monitor", SATFE_Command_monitor},
   {"ber", SATFE_Command_ber},
   {"reset_status", SATFE_Command_reset_status},
   {"rs", SATFE_Command_reset_status},
   {"chip", SATFE_Command_chip},
   {"channel", SATFE_Command_channel},
   {"chan", SATFE_Command_channel},
   {"symbol_rate", SATFE_Command_symbol_rate},
   {"Fb", SATFE_Command_symbol_rate},
   {"sr", SATFE_Command_symbol_rate},
   {"acq_ctl", SATFE_Command_acq_ctl},
   {"ac", SATFE_Command_acq_ctl},
   {"mode", SATFE_Command_mode},
   {"m", SATFE_Command_mode},
   {"carrier_offset", SATFE_Command_carrier_offset},
   {"repeat", SATFE_Command_repeat},
   {"test_acq", SATFE_Command_test_acq},
   {"switch", SATFE_Command_switch},
   {"monitor_lock_events", SATFE_Command_monitor_lock_events},
   {"dft_test", SATFE_Command_dft_test},
   {"plc_trk", SATFE_Command_plc_trk},
   {"plc_acq", SATFE_Command_plc_acq},
   {"test_status_indicators", SATFE_Command_test_status_indicators},
   {"search_range", SATFE_Command_search_range},
   {"tuner_filter", SATFE_Command_tuner_filter},
   {"debug", SATFE_Command_debug},
   {"blind_scan", SATFE_Command_blind_scan},
   {"blind_scan_psd", SATFE_Command_blind_scan_psd},
   {"xport", SATFE_Command_xport},
   {"xport_ctl", SATFE_Command_xport},
   {"config_diseqc", SATFE_Command_config_diseqc},
   {"network_spec", SATFE_Command_network_spec},
   {"check_timing_lock", SATFE_Command_check_timing_lock},
   {"search_tone", SATFE_Command_search_tone},
   {"power_down", SATFE_Command_power_down},
   {"power_up", SATFE_Command_power_up},
   {"test_power_down", SATFE_Command_test_power_down},
   {"vsense", SATFE_Command_vsense},
   {"test_send", SATFE_Command_test_send},
#ifdef BFSK_PROTOCOL_ECHO
   {"fsk_reset", SATFE_Command_fsk_reset},
   {"fsk_write", SATFE_Command_fsk_write},
   {"fsk_reprogram", SATFE_Command_fsk_reprogram},
   {"fsk_read", SATFE_Command_fsk_read},
   {"fsk_allocate", SATFE_Command_fsk_allocate},
   {"fsk_deallocate", SATFE_Command_fsk_deallocate},
   {"fsk_odu_reset", SATFE_Command_fsk_odu_reset},
   {"fsk_power_down", SATFE_Command_fsk_power_down},
   {"fsk_power_up", SATFE_Command_fsk_power_up},
   {"fsk_carrier", SATFE_Command_fsk_carrier},
   {"config_fsk", SATFE_Command_config_fsk},
   {"config_protocol", SATFE_Command_config_protocol},
   {"fsk_test_tdma", SATFE_Command_fsk_test_tdma},
#else
   {"ftm_uc_reset", SATFE_Command_ftm_uc_reset},
   {"ftm_write", SATFE_Command_ftm_write},
   {"ftm_read", SATFE_Command_ftm_read},
   {"ftm_uc_version", SATFE_Command_ftm_uc_version},
   {"ftm_options", SATFE_Command_ftm_options},
   {"ftm_rxmask", SATFE_Command_ftm_rxmask},
   {"ftm_hard_reset", SATFE_Command_ftm_hard_reset},
   {"ftm_register", SATFE_Command_ftm_register},
   {"ftm_echo", SATFE_Command_ftm_echo},
   {"ftm_ping", SATFE_Command_ftm_ping},
   {"ftm_xtune", SATFE_Command_ftm_xtune},
   {"ftm_lock", SATFE_Command_ftm_lock},
   {"ftm_unlock", SATFE_Command_ftm_unlock},
   {"ftm_test_register", SATFE_Command_ftm_test_register},
   {"ftm_test_xtune", SATFE_Command_ftm_test_xtune},
   {"ftm_test_echo", SATFE_Command_ftm_test_echo},
   {"ftm_power_down", SATFE_Command_ftm_power_down},
   {"ftm_power_up", SATFE_Command_ftm_power_up},
#endif
   {"bcm3445_config", SATFE_Command_bcm3445_config},
   {"bcm3445_map_output", SATFE_Command_bcm3445_map_output},
   {"bcm3445_status", SATFE_Command_bcm3445_status},
   {"bcm3445_read", SATFE_Command_bcm3445_read},
   {"bcm3445_write", SATFE_Command_bcm3445_write},
   {"config_gpio", SATFE_Command_config_gpio},
   {"get_gpio", SATFE_Command_get_gpio},
   {"set_gpio", SATFE_Command_set_gpio},
   {"gpio_config", SATFE_Command_config_gpio},
   {"gpio_get", SATFE_Command_get_gpio},
   {"gpio_set", SATFE_Command_set_gpio},
   {"freq_sweep", SATFE_Command_freq_sweep},
   {"cwc", SATFE_Command_cwc},
   {"regdump", SATFE_Command_regdump},
   {"delay", SATFE_Command_delay},
   {"mi2c_write", SATFE_Command_mi2c_write},
   {"mi2c_read", SATFE_Command_mi2c_read},
   {"reset_channel", SATFE_Command_reset_channel},
   {"soft_reset", SATFE_Command_soft_reset},
   {"pilot", SATFE_Command_pilot},
   {"bert", SATFE_Command_bert},
   {"amc_scrambling_seq", SATFE_Command_amc_scrambling_seq},
   {"get_snr_range", SATFE_Command_get_snr_range},
   {"adc", SATFE_Command_adc},
   {"diseqc_voltage", SATFE_Command_diseqc_voltage},
   {"diseqc_status", SATFE_Command_diseqc_status},
   {"test_mpeg_count", SATFE_Command_test_mpeg_count},
   {"test_lock_event", SATFE_Command_test_lock_event},
   {"test_snr", SATFE_Command_test_snr},
   {"lock_transponders", SATFE_Command_lock_transponders},
   {"dft_debug", SATFE_Command_dft_debug},
   {"reboot", SATFE_Command_reboot},
   {0, NULL},
};


/******************************************************************************
SATFE_Command_vsense()
******************************************************************************/
bool SATFE_Command_vsense(SATFE_Chip *pChip,int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t voltage;
   uint16_t lnbVoltage;
   bool bEnable;
   BDBG_ASSERT(pChip);
    if ((argc < 1) || (argc > 2))
   {
      SATFE_PrintDescription1("vsense", "vsense <on | off>", "enable / disable vsense interrupts", "on = enable vsense interrupts", false);
      SATFE_PrintDescription2("off = disable vsense interrupts", false);
      SATFE_PrintDescription2("(no option specified) = detect the LNB voltage level", true);
      return true;
   }

   if (pChip->commonCmds.vsense)
      return pChip->commonCmds.vsense(pChip, argc, argv);

   if (argc == 1)
   {
     /* detect LNB voltage level */
      SATFE_MUTEX(retCode = BAST_GetDiseqcVoltage(pChip->hAstChannel[pChip->currChannel], &voltage));
      /* printf("SAR ADC = %02X\n", voltage); */
      /* voltage = (uint8_t)(voltage - 0x80); */   /* convert to binary offset format */
      /* printf("   --> %02X\n", voltage); */
      if (retCode == BERR_SUCCESS)
      {
       if (pChip->chipFunctTable->GetAdcVoltage == NULL)
       {
             printf("GetAdcVoltage function not implemented for this chip!\n");
             return false;
       }
       if (pChip->chipFunctTable->GetAdcVoltage(pChip, voltage, (uint8_t)pChip->currChannel,&lnbVoltage))
            printf("estimated voltage: %d.%03d %d\n", lnbVoltage / 1000,lnbVoltage % 1000,lnbVoltage);
       else
       {
            printf("estimated voltage lookup failed\n");
            return false;
       }


       }
   }
   else
   {
      if (!strcmp(argv[1], "on"))
         bEnable = true;
      else if (!strcmp(argv[1], "off"))
         bEnable = false;
      else
      {
         printf("syntax error\n");
         return false;
      }
      /* enable/disable interrupt */
      SATFE_MUTEX(retCode = BAST_EnableVsenseInterrupts(pChip->hAstChannel[pChip->currChannel], bEnable));
   }

   SATFE_RETURN_ERROR("BAST_EnableVsenseInterrupts()", retCode);
}


/******************************************************************************
 SATFE_Open() - opens all AST handles for all frontend chips
******************************************************************************/
BERR_Code SATFE_Open(void *pParam)
{
   BAST_ChannelSettings chnSettings;
   BERR_Code retCode;
   uint32_t nChannels, i, j;
#ifdef SATFE_USE_BFSK
   BFSK_ChannelSettings fskChanSettings;
#endif

   g_pSATFE = (SATFE_Chip *)BKNI_Malloc(SATFE_NUM_CHIPS * sizeof(SATFE_Chip));
   BDBG_ASSERT(g_pSATFE);

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      g_pSATFE[i].idx = i;

      /* copy chip descriptor from platform to chip handle */
      BKNI_Memcpy((void*)&g_pSATFE[i].chip, (const void *)&SATFE_chips[i], sizeof(SATFE_ChipDescriptor));

      /* allocate memory for acq settings */
      g_pSATFE[i].acqSettings = (BAST_AcqSettings*)BKNI_Malloc(SATFE_chips[i].nDemods * 2 * sizeof(BAST_AcqSettings));

      /* open ast */
      if ((retCode = g_pSATFE[i].chip.platformFunctTable->Open((struct SATFE_Chip*)&g_pSATFE[i], pParam)) != BERR_SUCCESS)
      {
         printf("Open() error 0x%X\n", retCode);
         goto done;
      }

      BAST_GetTotalChannels(g_pSATFE[i].hAst, &nChannels);

      /* open the channel handles */
      if (g_pSATFE[i].chip.platformFunctTable->OpenChannels)
      {
         retCode = g_pSATFE[i].chip.platformFunctTable->OpenChannels((struct SATFE_Chip*)&g_pSATFE[i], pParam);
         if (retCode != BERR_SUCCESS)
         {
            printf("OpenChannels() error 0x%X\n", retCode);
            goto done;
         }
      }
      else
      {
         /* standard method to open channels */
         g_pSATFE[i].hAstChannel = (BAST_ChannelHandle *)BKNI_Malloc(nChannels * sizeof(BAST_ChannelHandle));
         for (j = 0; j < nChannels; j++)
         {
            BAST_GetChannelDefaultSettings(g_pSATFE[i].hAst, j, &chnSettings);
            retCode = BAST_OpenChannel(g_pSATFE[i].hAst, &(g_pSATFE[i].hAstChannel[j]), j, &chnSettings);
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_OpenChannel() error 0x%X\n", retCode);
               BKNI_Free((void*)g_pSATFE[i].hAstChannel);
               goto done;
            }
         }
      }

#ifdef SATFE_USE_BFSK
      /* open FSK PI channels */
      BFSK_GetTotalChannels(g_pSATFE[i].hFsk, &(g_pSATFE[i].nFskChannels));
      g_pSATFE[i].hFskChannel = (BFSK_ChannelHandle*)BKNI_Malloc(g_pSATFE[i].nFskChannels * sizeof(BFSK_ChannelHandle));
      for (j = 0; j < g_pSATFE[i].nFskChannels; j++)
      {
         BFSK_GetChannelDefaultSettings(g_pSATFE[i].hFsk, j, &fskChanSettings);
         retCode = BFSK_OpenChannel(g_pSATFE[i].hFsk, &(g_pSATFE[i].hFskChannel[j]), j, &fskChanSettings);
         if (retCode != BERR_SUCCESS)
         {
            printf("BFSK_OpenChannel(%d) error 0x%X\n", j, retCode);
            BKNI_Free((void*)g_pSATFE[i].hFskChannel);
            goto done;
         }
      }
#endif
   }

   done:
   if (retCode != BERR_SUCCESS)
   {
      BKNI_Free((void*)g_pSATFE);
      g_pSATFE = NULL;
   }

   return retCode;
}


/******************************************************************************
 SATFE_Close()
******************************************************************************/
BERR_Code SATFE_Close(void)
{
   SATFE_Chip *pChip;
   int i, j;

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      pChip = SATFE_GetChipByIndex(i);
      if (pChip->hAstChannel)
      {
         for (j = 0; j < pChip->chip.nDemods; j++)
         {
            if (pChip->hAstChannel[j])
            {
               BAST_CloseChannel(pChip->hAstChannel[j]);
               pChip->hAstChannel[j] = NULL;
            }
         }
      }

      if (pChip->hAst)
      {
         if (g_pSATFE[i].chip.platformFunctTable->Close)
            g_pSATFE[i].chip.platformFunctTable->Close(pChip);
         else
            BAST_Close(pChip->hAst);
         BKNI_Free(pChip->hAstChannel);
         pChip->hAst = NULL;
         pChip->hAstChannel = NULL;
      }
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_Init() - initializes all the frontend chips
******************************************************************************/
BERR_Code SATFE_Init(void *pParam, bool bInitAp)
{
   BERR_Code  retCode;
   SATFE_Chip *pChip;
   int        i, j;
   uint32_t   bondId;
   uint16_t   chipId;
   uint8_t    fwVer, cfgVer, chipVer;
   char       str[16], *argv[2];

   SATFE_Platform_Init(pParam);

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      pChip = SATFE_GetChipByIndex(i);

      /* default handle initialization */
      pChip->pImpl = NULL;
      for (j = 0; j < pChip->chip.nDemods; j++)
      {
         pChip->currAcqSettings = (j * 2);
#if 0
         pChip->acqSettings[pChip->currAcqSettings].mode = BAST_Mode_eTurbo_8psk_2_3;
         pChip->acqSettings[pChip->currAcqSettings].symbolRate = 21500000;
#else
         pChip->acqSettings[pChip->currAcqSettings].mode = BAST_Mode_eDvb_scan;
         pChip->acqSettings[pChip->currAcqSettings].symbolRate = 20000000;
#endif
         pChip->acqSettings[pChip->currAcqSettings].acq_ctl = (BAST_ACQSETTINGS_DEFAULT & ~BAST_ACQSETTINGS_LDPC_PILOT_PLL);
         pChip->acqSettings[pChip->currAcqSettings].carrierFreqOffset = 0;
         pChip->acqSettings[pChip->currAcqSettings + 1].mode = BAST_Mode_eDss_scan;
         pChip->acqSettings[pChip->currAcqSettings + 1].acq_ctl = (BAST_ACQSETTINGS_DEFAULT & ~BAST_ACQSETTINGS_LDPC_PILOT_PLL);
         pChip->acqSettings[pChip->currAcqSettings + 1].symbolRate = 20000000;
         pChip->acqSettings[pChip->currAcqSettings + 1].carrierFreqOffset = 0;
      }
      pChip->currChannel = 0;
      pChip->currAcqSettings = 0;
      pChip->currFskChannel = 0;
      pChip->pFwImage = NULL;
      pChip->pPlatformCommandSet = NULL;
      pChip->hMutex = NULL;
      pChip->hFtmMessageMutex = NULL;
      pChip->bEnableFtmLogging = true;
      pChip->ftmPacketCount = 0;
      pChip->bInit = false;
      pChip->amcScramblingIdx[0] = pChip->amcScramblingIdx[1] = 0;
      BKNI_Memset(&(pChip->commonCmds), 0, sizeof(SATFE_CommandMap));
      pChip->chip.platformFunctTable->InitHandle((struct SATFE_Chip*)&g_pSATFE[i], pParam);

      BKNI_CreateMutex(&(pChip->hFtmMessageMutex));
      BKNI_CreateEvent(&(pChip->hFtmMessageEvent));

      argv[0] = "debug";
      argv[1] = "on";
      SATFE_Command_debug(pChip, 2, argv);

      if (bInitAp)
      {
         SATFE_MUTEX(retCode = BAST_InitAp(pChip->hAst, pChip->pFwImage));
         SATFE_CHECK_RETCODE("BAST_InitAp()", retCode);

         pChip->bInit = true;

         SATFE_MUTEX(retCode = BAST_GetApVersion(pChip->hAst, &chipId, &chipVer, &bondId, &fwVer, &cfgVer));
         SATFE_CHECK_RETCODE("BAST_GetApVersion()", retCode);
         if (pChip->chip.name == NULL)
         {
            sprintf(str, "BCM%04X-%02X", chipId, chipVer);
            pChip->chip.name = BKNI_Malloc(sizeof(str) + 1);
            strcpy(pChip->chip.name, str);
         }
         printf("*** %s started: FW version %02X.%02X\n",
                pChip->chip.name, fwVer, cfgVer);
      }

      if (pChip->chip.platformFunctTable->Configure != NULL)
      {
         /* setup platform configurations */
         pChip->chip.platformFunctTable->Configure((struct SATFE_Chip*)pChip, pParam);
      }

      argv[0] = "debug";
      argv[1] = "off";
      SATFE_Command_debug(pChip, 2, argv);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_Shutdown()
******************************************************************************/
BERR_Code SATFE_Shutdown(void)
{
   BERR_Code retCode;
   SATFE_Chip *pChip;
   int i;

   retCode = SATFE_Platform_Shutdown();
   if (retCode)
      printf("SATFE_Platform_Shutdown() error 0x%X\n", retCode);

   retCode = SATFE_Close();

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      pChip = SATFE_GetChipByIndex(i);

      BKNI_DestroyMutex(pChip->hFtmMessageMutex);
      BKNI_DestroyEvent(pChip->hFtmMessageEvent);
   }

   return retCode;
}


/******************************************************************************
 SATFE_LookupCommand()
******************************************************************************/
bool SATFE_LookupCommand(SATFE_Chip *pChip, char *cmdstr, SATFE_CommandFunct *funct)
{
   int i;

   if (pChip->pPlatformCommandSet)
   {
      for (i = 0; pChip->pPlatformCommandSet[i].name; i++)
      {
         if (!strcmp(cmdstr, (const char *)pChip->pPlatformCommandSet[i].name))
         {
            *funct = pChip->pPlatformCommandSet[i].funct;
            return true;
         }
      }
   }

   if (pChip->pChipCommandSet)
   {
      for (i = 0; pChip->pChipCommandSet[i].name; i++)
      {
         if (!strcmp(cmdstr, (const char *)pChip->pChipCommandSet[i].name))
         {
            *funct = pChip->pChipCommandSet[i].funct;
            return true;
         }
      }
   }

   for (i = 0; SATFE_CommandSet[i].name; i++)
   {
      if (!strcmp(cmdstr, (const char *)SATFE_CommandSet[i].name))
      {
         *funct = SATFE_CommandSet[i].funct;
         return true;
      }
   }

   *funct = NULL;
   return false;
}


/******************************************************************************
 SATFE_get_string()
******************************************************************************/
void SATFE_get_string(char *cmdline)
{
   char input_char, *s_ptr;
   int n_chars;

   fflush(stdout);

   /* get the user command input */
   n_chars = 0;
   input_char = 0;
   s_ptr = cmdline;
   while (n_chars < 256)
   {
      input_char = SATFE_Platform_GetChar(true);

      if ((input_char == '\r') || (input_char == '\n'))
         break;
      else if (input_char == 0x08)
      {
         if (n_chars > 0)
         {
            *s_ptr-- = 0;
            n_chars--;
            SATFE_Platform_Backspace();
         }
      }
      else
      {
         *s_ptr++ = input_char;
         n_chars++;
      }
   }
   printf("\n");
   *s_ptr = 0;
}


/******************************************************************************
 SATFE_Diags()
******************************************************************************/
int SATFE_Diags(void *pParam, int bInitAp)
{
   static bool bInit = false;
   bool bQuit;
   BERR_Code retCode;
   SATFE_Chip *pChip;
   uint32_t i;
   int argc;
   char cmdline[256], last_command[256], *argv[64], str[16];

   if (!bInit)
   {
      retCode = SATFE_Open(pParam);
      if (retCode != BERR_SUCCESS)
         return 0;

      retCode = SATFE_Init(pParam, bInitAp);
      if (retCode != BERR_SUCCESS)
      {
         printf("Unable to initialize AST PI\n");
         SATFE_Shutdown();
         return 0;
      }
      SATFE_Platform_InitDiags(pParam);
      bInit = true;
   }

   last_command[0] = 0;
   for (bQuit = false; !bQuit; )
   {
      pChip = SATFE_GetCurrChip();
      if (pChip->chip.name)
         strcpy(str, pChip->chip.name);
      else
         sprintf(str, "chip%d", pChip->idx);
      printf("\n%s:%d> ", str, pChip->currChannel);
      SATFE_get_string(cmdline);

      if (!strcmp(cmdline, "!!"))
      {
         strcpy(cmdline, last_command);
         printf("%s\n\n", cmdline);
      }
      strcpy(last_command, cmdline);

      /* make every character lowercase */
      for (i = 0; i < strlen(cmdline); i++)
         cmdline[i] = tolower(cmdline[i]);

      argv[0] = strtok(cmdline, (const char *)" \t\n");
      if (!argv[0])
         continue;

      /* get the rest of the tokens */
      for (argc = 1; ; argc++)
      {
         argv[argc] = strtok((char*)NULL, (const char *)" \t\n");
         if (!argv[argc])
            break;
      }

      if (!strcmp((const char *)argv[0], (const char *)"quit") ||
          !strcmp((const char *)argv[0], (const char *)"exit"))
      {
         bQuit = 1;
         continue;
      }

      SATFE_ProcessCommand(pChip, argc, argv);
   }

   if (bInit)
      return 1;
   else
      return 0;
}


/******************************************************************************
 SATFE_ProcessCommand()
******************************************************************************/
void SATFE_ProcessCommand(SATFE_Chip *pChip, int argc, char **argv)
{
   SATFE_CommandFunct funct;

   if (SATFE_LookupCommand(pChip, argv[0], &funct))
   {
      if (funct((struct SATFE_Chip*)SATFE_GetCurrChip(), argc, argv) == false)
         printf("ERROR - command not performed\n");
   }
   else
      printf("invalid command: %s\n", argv[0]);
}


/******************************************************************************
 SATFE_GetChipByIndex()
******************************************************************************/
SATFE_Chip* SATFE_GetChipByIndex(int idx)
{
   return &g_pSATFE[idx];
}


/******************************************************************************
 SATFE_GetCurrChip()
******************************************************************************/
SATFE_Chip* SATFE_GetCurrChip(void)
{
   SATFE_Chip *pChip = NULL;

   if (g_pSATFE)
      pChip = SATFE_GetChipByIndex(g_SATFE_curr_chip_idx);
   return pChip;
}


/******************************************************************************
 SATFE_LookupRegister()
******************************************************************************/
bool SATFE_LookupRegister(SATFE_Chip *pChip, char *name, SATFE_Register **pReg)
{
   int i, j;
   char regname[80];

   for (i = 0; pChip->regMap[i].name; i++)
   {
      strcpy(regname, pChip->regMap[i].name);
      for (j = 0; j < (int)strlen(regname); j++)
         regname[j] = tolower(regname[j]);
      if (!strcmp(name, regname))
      {
         *pReg = &(pChip->regMap[i]);
         return true;
      }
   }
   return false;
}


/******************************************************************************
 SATFE_LookupConfigParam()
******************************************************************************/
bool SATFE_LookupConfigParam(SATFE_Chip *pChip, char *name, SATFE_ConfigParam **pParam)
{
   int i;

   for (i = 0; pChip->configParamMap[i].name; i++)
   {
      if (!strcmp(name, pChip->configParamMap[i].name))
     {
        *pParam = &(pChip->configParamMap[i]);
        return true;
     }
   }
   return false;
}


/******************************************************************************
 SATFE_PrintDescription2()
******************************************************************************/
void SATFE_PrintDescription2(char *option, bool bEnd)
{
   printf("   %s\n", option);
   if (bEnd)
      printf("------------------------------------------------------------------------------\n");
}


/******************************************************************************
 SATFE_PrintDescription1()
******************************************************************************/
void SATFE_PrintDescription1(char *cmd, char *syntax, char *desc, char *option, bool bEnd)
{
   printf("------------------------------------------------------------------------------\n");
   printf("COMMAND: %s\n", cmd);
   printf("SYNTAX:\n");
   printf("   %s\n", syntax);
   printf("DESCRIPTION:\n");
   printf("   %s\n", desc);
   printf("INPUT PARAMETERS / OPTIONS:\n");
   SATFE_PrintDescription2(option, bEnd);
}


/******************************************************************************
 SATFE_IsSpinv()
******************************************************************************/
bool SATFE_IsSpinv(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus)
{
   if (pChip->chipFunctTable->IsSpinv)
      return pChip->chipFunctTable->IsSpinv(pChip, pStatus);
   else
      return (pStatus->modeStatus.ldpc.hpstatus & 0x00020000) ? true : false;
}


/******************************************************************************
 SATFE_IsPilotOn()
******************************************************************************/
bool SATFE_IsPilotOn(SATFE_Chip *pChip, BAST_ChannelStatus *pStatus)
{
   if (pChip->chipFunctTable->IsPilotOn)
      return pChip->chipFunctTable->IsPilotOn(pChip, pStatus);
   else
      return (pStatus->modeStatus.ldpc.hpstatus & 0x80000000) ? true : false;
}


/******************************************************************************
 SATFE_GetFreqFromString()
******************************************************************************/
bool SATFE_GetFreqFromString(SATFE_Chip *pChip, char *str, uint32_t *pHz)
{
   float f;
   uint32_t hz;

   if (pChip->chipFunctTable->GetFreqFromString)
      return pChip->chipFunctTable->GetFreqFromString(str, pHz);
   else
   {
      f = (float)atof(str);
      if (f < 2200)
         hz = (uint32_t)(f * 1000000);
      else
         hz = (uint32_t)f;

      if ((hz < 250000000UL) || (hz > 2150000000UL))
      {
         printf("Tuning frequency out of range\n");
         return false;
      }
      *pHz = hz;
   }
   return true;
}


/******************************************************************************
 SATFE_GetSymbolRateFromString()
******************************************************************************/
bool SATFE_GetSymbolRateFromString(SATFE_Chip *pChip, char *str, uint32_t *pFb)
{
   float f;
   uint32_t Fb;

   BSTD_UNUSED(pChip);

   f = (float)atof(str);
   if (f < 45)
      Fb = (uint32_t)(f * 1000000);
   else
      Fb = (uint32_t)f;

   if ((Fb < 250000) || (Fb > 45000000))
   {
      printf("Symbol rate out of range\n");
      return false;
   }
   *pFb = Fb;
   return true;
}


/******************************************************************************
 SATFE_GetConfigParamValueString()
******************************************************************************/
void SATFE_GetConfigParamValueString(SATFE_Chip *pChip, char *str, SATFE_ConfigParam *pParam)
{
   BERR_Code retCode;
   uint8_t buf[16];
   char val_str[16];

   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pParam->id, buf, pParam->len));
   if (retCode == BERR_SUCCESS)
   {
      switch (pParam->len)
      {
         case 1:
            sprintf(val_str, "0x%02X", buf[0]);
            break;
         case 2:
            sprintf(val_str, "0x%02X%02X", buf[0], buf[1]);
            break;
         case 4:
            sprintf(val_str, "0x%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3]);
            break;
         case 16:
            sprintf(val_str, "0x%02X%02X%02X%02X...", buf[0], buf[1], buf[2], buf[3]);
            break;
         default:
            sprintf(val_str, "?");
            break;
      }
   }
   else
      strcpy(val_str, "<error>");
   sprintf(str, "%22s = %-14s", pParam->name, val_str);
}


/******************************************************************************
 SATFE_ValidateHexString()
******************************************************************************/
bool SATFE_ValidateHexString(char *str, int len)
{
   int i;

   if ((int)strlen(str) > len)
      goto bad_string;

   for (i = 0; i < (int)strlen(str); i++)
   {
      if (!(((str[i] >= '0') && (str[i] <= '9')) || ((str[i] >= 'a') && (str[i] <= 'f'))))
      {
         bad_string:
         printf("Invalid hexadecimal string\n");
         return false;
      }
   }

   return true;
}


/******************************************************************************
 SATFE_ValidateDecString()
******************************************************************************/
bool SATFE_ValidateDecString(char *str, int bits)
{
   uint32_t val, max_val;
   int i;

   if (strlen(str) > 10)
      goto bad_string;

   for (i = 0; i < (int)strlen(str); i++)
   {
      if (!((str[i] >= '0') && (str[i] <= '9')))
      {
         bad_string:
         printf("Invalid decimal string\n");
         return false;
      }
   }

   if (bits < 32)
   {
      max_val = (1 << bits) - 1;
      val = strtoul(str, NULL, 10);
      if (val > max_val)
      {
         printf("number out of range\n");
         return false;
      }
   }

   return true;
}


/******************************************************************************
 SATFE_GetU8FromString()
******************************************************************************/
bool SATFE_GetU8FromString(char *str, uint8_t *pVal)
{
   if (strlen(str) > 2)
   {
      if (!strncmp(str, "0x", 2))
      {
         if (SATFE_ValidateHexString(&str[2], 2))
         {
            *pVal = (uint8_t)strtoul(&str[2], NULL, 16);
            return true;
         }
         else
            return false;
      }
   }

   if (SATFE_ValidateDecString(str, 8))
   {
      *pVal = (uint8_t)strtoul(str, NULL, 10);
      return true;
   }

   return false;
}


/******************************************************************************
 SATFE_GetU16FromString()
******************************************************************************/
bool SATFE_GetU16FromString(char *str, uint16_t *pVal)
{
   if (strlen(str) > 2)
   {
      if (!strncmp(str, "0x", 2))
      {
         if (SATFE_ValidateHexString(&str[2], 4))
         {
            *pVal = (uint16_t)strtoul(&str[2], NULL, 16);
            return true;
         }
         else
            return false;
      }
   }

   if (SATFE_ValidateDecString(str, 16))
   {
      *pVal = (uint16_t)strtoul(str, NULL, 10);
      return true;
   }

   return false;
}


/******************************************************************************
 SATFE_GetU32FromString()
******************************************************************************/
bool SATFE_GetU32FromString(char *str, uint32_t *pVal)
{
   if (strlen(str) > 2)
   {
      if (!strncmp(str, "0x", 2))
      {
         if (SATFE_ValidateHexString(&str[2], 8))
         {
            *pVal = (uint32_t)strtoul(&str[2], NULL, 16);
            return true;
         }
         else
            return false;
      }
   }

   if (SATFE_ValidateDecString(str, 32))
   {
      *pVal = (uint32_t)strtoul(str, NULL, 10);
      return true;
   }

   return false;
}


/******************************************************************************
 SATFE_GetModeString()
******************************************************************************/
char* SATFE_GetModeString(BAST_Mode mode)
{
   int i;
   static char *str = "unknown";

   for (i = 0; SATFE_ModeList[i].name; i++)
   {
      if (SATFE_ModeList[i].mode == mode)
      {
         str = SATFE_ModeList[i].name;
         break;
      }
   }

   return str;
}


/******************************************************************************
 SATFE_GetString()
******************************************************************************/
void SATFE_GetString(char *cmdline)
{
   char input_char, *s_ptr;
   int n_chars;

   fflush(stdout);

   /* get the user command input */
   n_chars = 0;
   input_char = 0;
   s_ptr = cmdline;
   while (n_chars < 256)
   {
      input_char = SATFE_Platform_GetChar(true);

      if ((input_char == '\r') || (input_char == '\n'))
         break;
      else if (input_char == 0x08)
      {
         if (n_chars > 0)
         {
            *s_ptr-- = 0;
            n_chars--;
            SATFE_Platform_Backspace();
         }
      }
      else
      {
         *s_ptr++ = input_char;
         n_chars++;
      }
   }
   printf("\n");
   *s_ptr = 0;
}


/******************************************************************************
 SATFE_WaitForKeyPress()
******************************************************************************/
char SATFE_WaitForKeyPress(uint32_t timeout)
{
   char key = 0;

   SATFE_Platform_StartTimer();
   while (SATFE_Platform_GetTimerCount() < timeout)
   {
      key = SATFE_Platform_GetChar(false);
      if (key > 0)
         break;
      SATFE_OnIdle();
   }
   SATFE_Platform_KillTimer();
   return key;
}


/******************************************************************************
 SATFE_IsFtmInit() - returns true if BAST_ResetFtm() was called
******************************************************************************/
bool SATFE_IsFtmInit(SATFE_Chip *pChip)
{
   BSTD_UNUSED(pChip);
   return ResetStatus;
}


/******************************************************************************
 SATFE_GetSoftDecisions()
******************************************************************************/
bool SATFE_GetSoftDecisions(SATFE_Chip *pChip, int channel, short *iBuf, short *qBuf)
{
   BERR_Code retCode;

   SATFE_MUTEX(retCode = BAST_GetSoftDecisionBuf(pChip->hAstChannel[channel], iBuf, qBuf));
   return (retCode == BERR_SUCCESS ? true : false);
}


/******************************************************************************
 SATFE_GetStatusItem()
******************************************************************************/
bool SATFE_GetStatusItem(SATFE_Chip *pChip, int chn, int statusItem, void *pStatus)
{
   BERR_Code retCode;
   BAST_ChannelStatus status;
   float *pFloat = (float*)pStatus;
   bool *pBool = (bool*)pStatus;

   SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[chn], &status));
   if (retCode)
   {
      printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
      return false;
   }

   switch (statusItem)
   {
      case SATFE_STATUS_SNR:
         *pFloat = (float)(status.snrEstimate / 256.0);
         break;

      case SATFE_STATUS_INPUT_POWER:
         SATFE_Platform_GetInputPower(pChip, status.RFagc, status.IFagc, (uint32_t)status.agf,
                                      status.tunerFreq, pFloat);

         break;

      case SATFE_STATUS_CARRIER_ERROR:
         *pFloat = (float)status.carrierError;
         break;

      case SATFE_STATUS_LOCK:
         *pBool = status.bDemodLocked ? 1 : 0;
         break;

      default:
         printf("Error in SATFE_GetStatusItem(): invalid status item %d\n", statusItem);
         break;
   }

   return true;
}


/******************************************************************************
 SATFE_TuneAcquire()
******************************************************************************/
BERR_Code SATFE_TuneAcquire(SATFE_Chip *pChip, uint32_t freq, BAST_AcqSettings *pAcqSettings)
{
   BERR_Code retCode;

   if (pAcqSettings == NULL)
      pAcqSettings = &(pChip->acqSettings[pChip->currAcqSettings]);

   if (pChip->chip.platformFunctTable->TuneAcquire)
   {
      /* call platform specific tune acquire if available */
      retCode = pChip->chip.platformFunctTable->TuneAcquire(pChip, freq, pAcqSettings);
   }
   else
   {
      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], freq, pAcqSettings));
      if (retCode)
         printf("BAST_TuneAcquire() error 0x%X\n", retCode);
   }
   return retCode;
}


/******************************************************************************
 SATFE_TimedAcquire()
******************************************************************************/
/* #define SATFE_TIMED_ACQ_USE_EXTERNAL_TIMER */
void SATFE_TimedAcquire(SATFE_Chip *pChip, uint32_t tuner_freq, BAST_AcqSettings *pSettings, uint32_t lockTimeout, uint32_t lockStableTime, bool *pAborted, bool *pLocked, BAST_ChannelStatus *pStatus, uint32_t *pAcqTime)
{
   BERR_Code retCode;
   SATFE_ConfigParam *pParam;
   uint32_t curr_time, time0;
   bool bLocked, bFirstTimeLocked;
#ifndef SATFE_TIMED_ACQ_USE_EXTERNAL_TIMER
   uint8_t buf[4];
#endif

   *pAborted = false;
   *pLocked = false;
   *pAcqTime = 0;
   bFirstTimeLocked = false;
   time0 = 0;

   if (SATFE_LookupConfigParam(pChip, "acq_time", &pParam) == false)
   {
      printf("Unable to find acq_time configuration parameter\n");
      goto abort;
   }

   if (SATFE_TuneAcquire(pChip, tuner_freq, pSettings) != BERR_SUCCESS)
   {
      abort:
      *pAborted = true;
      goto done;
   }
   SATFE_Platform_StartTimer();

   while (1)
   {
      curr_time = SATFE_Platform_GetTimerCount();
      SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked));
      if (retCode)
      {
         printf("BAST_GetLockStatus() error 0x%X\n", retCode);
         goto abort;
      }
      if (bLocked)
      {
         if (!bFirstTimeLocked)
         {
            bFirstTimeLocked = true;
            time0 = curr_time;
         }
         else if ((curr_time - time0) >= lockStableTime)
         {
            /* stable lock */
#ifdef SATFE_TIMED_ACQ_USE_EXTERNAL_TIMER
            *pAcqTime = time0;
#else
            SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pParam->id, buf, pParam->len));
            if (retCode)
            {
               printf("BAST_ReadConfig(%s) error 0x%X\n", pParam->name, retCode);
               goto abort;
            }
            *pAcqTime = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
            *pAcqTime = (*pAcqTime + 500) / 1000; /* round to msecs */
#endif

            SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], pStatus));
            if (retCode)
            {
               printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
               goto abort;
            }
            if (!pStatus->bDemodLocked)
               goto not_locked;

            *pLocked = true;
            break;
         }
      }
      else
      {
         not_locked:
         if (bFirstTimeLocked)
            printf("fell out of lock\n");
         bFirstTimeLocked = false;
         if (curr_time > lockTimeout)
            break;
      }

   #ifndef SOAP
      if (SATFE_Platform_GetChar(false) > 0)
         goto abort;
   #endif
      SATFE_OnIdle();
   }

   done:
   SATFE_Platform_KillTimer();
}


/******************************************************************************
 SATFE_WriteAmcScramblingSeq()
******************************************************************************/
bool SATFE_WriteAmcScramblingSeq(SATFE_Chip *pChip, int idx)
{
   BERR_Code retCode;
   SATFE_AmcScramblingSeq *pSeq;

   pSeq = &SATFE_ScramblingSeqList[idx];
   SATFE_MUTEX(retCode = BAST_SetAmcScramblingSeq(pChip->hAstChannel[pChip->currChannel], pSeq->xseed, pSeq->plhdrscr1, pSeq->plhdrscr2, pSeq->plhdrscr3));
   if (retCode)
   {
      printf("BAST_SetAmcScramblingSeq() error 0x%X\n", retCode);
      return false;
   }
   else
   {
      pChip->amcScramblingIdx[pChip->currAcqSettings] = idx;
      printf("   xseed     = 0x%08X\n" ,pSeq->xseed);
      printf("   plhdrscr1 = 0x%08X\n", pSeq->plhdrscr1);
      printf("   plhdrscr2 = 0x%08X\n", pSeq->plhdrscr2);
      printf("   plhdrscr3 = 0x%08X\n", pSeq->plhdrscr3);
   }
   return true;
}


/******************************************************************************
 SATFE_GetCrc8()
******************************************************************************/
uint8_t SATFE_GetCrc8(uint8_t p, uint8_t c, uint8_t b)
{
   uint8_t i;
   bool fTemp1;

   for (i = 0; i < 8; i++)
   {
      fTemp1 = false;
      if (c & 0x80)
         fTemp1 = true;
      c <<= 1;
      if (b & 0x80)
         c ^= 1;
      if (fTemp1)
         c ^= p;
      b <<= 1;
   }
   return c;
}


/******************************************************************************
 SATFE_GetStreamCrc8()
******************************************************************************/
uint8_t SATFE_GetStreamCrc8(uint8_t *pBuf, int n)
{
   int i;
   uint8_t p, c;

   p = 0x4D;
   c = 0xFF;
   for (i = 0; i < n; i++)
      c = SATFE_GetCrc8(p, c, pBuf[i]);
   return c;
}


/******************************************************************************
 SATFE_OnIdle()
******************************************************************************/
void SATFE_OnIdle()
{
   BKNI_EventHandle hFtmEvent;
   BERR_Code retCode;
   SATFE_Chip *pChip;
   int i;
   uint8_t rxBuf[65], rxLen;
#ifdef SATFE_USE_BFSK
   uint8_t rxBytesUnread;
#endif

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      pChip = SATFE_GetChipByIndex(i);
   #ifdef SATFE_USE_BFSK
      retCode = BFSK_GetRxEventHandle(pChip->hFskChannel[0], &hFtmEvent);
      if (retCode != BERR_SUCCESS)
      {
         printf("BFSK_GetRxEventHandle() error 0x%X\n", retCode);
         continue;
      }
   #else
      retCode = BAST_GetFtmEventHandle(pChip->hAst, &hFtmEvent);
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetFtmEventHandle() error 0x%X\n", retCode);
         continue;
      }
   #endif

      if (hFtmEvent == NULL)
         continue;

      /* check for ftm message - should not be any ftm messages while data is transmitting */
      if (BKNI_WaitForEvent(hFtmEvent, 0) == BERR_SUCCESS)
      {
         /* pending FTM messages */
      #ifdef SATFE_USE_BFSK
         SATFE_MUTEX(retCode = BFSK_Read(pChip->hFskChannel[0], rxBuf, 65, &rxLen, &rxBytesUnread));
      #else
         SATFE_MUTEX(retCode = BAST_ReadFtm(pChip->hAst, rxBuf, &rxLen));
      #endif
         if (retCode == BERR_SUCCESS)
         {
            if (rxLen > 0)
               pChip->ftmPacketCount++;

            if (BKNI_AcquireMutex(pChip->hFtmMessageMutex) == BERR_SUCCESS)
            {
               BKNI_Memcpy(pChip->ftmMessageBuf, rxBuf, rxLen);
               BKNI_ReleaseMutex(pChip->hFtmMessageMutex);
               BKNI_SetEvent(pChip->hFtmMessageEvent);
            }
            SATFE_Ftm_LogRead(pChip, rxBuf, rxLen);
         }
         else
            printf("BAST_ReadFtm() error\n");
      }
   }

   if (pChip->chip.platformFunctTable->OnIdle)
   {
      /* call platform specific idle function if available */
      pChip->chip.platformFunctTable->OnIdle();
   }
}


/******************************************************************************
 SATFE_Command_help()
******************************************************************************/
bool SATFE_Command_help(SATFE_Chip *pChip, int argc, char **argv)
{
   SATFE_CommandFunct funct;

   BDBG_ASSERT(pChip);

   if (argc != 2)
   {
      printf("--------------------------------- COMMANDS -----------------------------------\n");
      printf("ACQUISITION COMMANDS:\n");
      printf("   ta, symbol_rate, mode, carrier_offset, acq_ctl, config, chip, channel,\n");
      printf("   abort, switch, search_range, blind_scan, xport, search_tone, reset_channel,\n");
      printf("   pilot, bert, amc_scrambling_seq, adc\n");
      printf("\n");
      printf("STATUS COMMANDS:\n");
      printf("   status, reset_status, monitor, ber, ver\n");
      printf("\n");
      printf("DISEQC COMMANDS:\n");
      printf("   diseqc_reset, vbot, vtop, tone, send, config_diseqc, diseqc_voltage,\n");
      printf("   diseqc_status\n");
      printf("\n");
   #ifdef BFSK_PROTOCOL_ECHO
      printf("FSK COMMANDS:\n");
      printf("   fsk_reset, fsk_write, fsk_read, config_fsk,\n");
      printf("   fsk_power_down, fsk_power_up\n");
      printf("\n");
   #else
      printf("SWM COMMANDS:\n");
      printf("   ftm_uc_reset, ftm_write, ftm_read, ftm_uc_version, ftm_options, ftm_rxmask,\n");
      printf("   ftm_hard_reset, ftm_register, ftm_echo, ftm_ping, ftm_xtune, ftm_lock,\n");
      printf("   ftm_unlock, ftm_test_register, ftm_test_xtune, ftm_power_down, ftm_power_up\n");
      printf("\n");
   #endif
      printf("PERIPHERAL CONTROL COMMANDS:\n");
      printf("   bcm3445_config, bcm3445_map_output, bcm3445_status, bcm3445_read,\n");
      printf("   bcm3445_write, gpio_config, gpio_set, gpio_get, mi2c_write, mi2c_read\n");
      printf("\n");
      printf("TEST/DIAGNOSTICS COMMANDS:\n");
      printf("   read, write, test_acq, debug, repeat, monitor_lock_events, dft_test,\n");
      printf("   plc_acq, plc_trk, test_status_indicators, tuner_filter, network_spec,\n");
      printf("   check_timing_lock, power_down, power_up, cwc, regdump, delay, soft_reset,\n");
      printf("   get_snr_range, test_mpeg_count, test_lock_event\n");
      printf("\n");

      if (pChip->commonCmds.help)
         pChip->commonCmds.help(pChip, argc, argv);

      if (SATFE_LookupCommand(pChip, "_help", &funct))
         funct(pChip, argc, argv);

      printf("enter 'help [command]' for more information\n");
      printf("------------------------------------------------------------------------------\n");
   }
   else
   {
      if (SATFE_LookupCommand(pChip, argv[1], &funct))
      {
         funct(pChip, 0, NULL);
      }
      else
         printf("unknown command: %s\n", argv[1]);
   }
   return true;
}


/******************************************************************************
 SATFE_Command_regdump()
******************************************************************************/
bool SATFE_Command_regdump(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int i, j;
   uint32_t uval32;
   bool bCore = false;
   char regname[80], corename[80];

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("regdump", "regdump <prefix>", "Dump out register values.", "prefix = dump out those registers whose name begins with the given prefix (e.g. sds_cl)", true);
      return true;
   }

   if (argc == 2)
   {
      bCore = true;
      for (j = 0; j < (int)strlen(argv[1]); j++)
         corename[j] = tolower(argv[1][j]);
      corename[j] = 0;
   }

   for (i = 0; pChip->regMap[i].name; i++)
   {
      if (pChip->regMap[i].type != SATFE_RegisterType_eISB)
         continue;

      strcpy(regname, pChip->regMap[i].name);
      for (j = 0; j < (int)strlen(regname); j++)
         regname[j] = tolower(regname[j]);

      if (bCore)
      {
         if (strncmp(corename, regname, strlen(corename)))
            continue;
      }

      SATFE_MUTEX(retCode = BAST_ReadRegister(pChip->hAstChannel[pChip->currChannel], pChip->regMap[i].addr, &uval32));
      if (retCode == BERR_SUCCESS)
      {
         /*printf("%s = 0x%08X (%u) %s\n", pChip->regMap[i].name, uval32, uval32, SATFE_get_32bit_binary_string(uval32));*/
         printf("%s = 0x%08X (%u)\n", pChip->regMap[i].name, uval32, uval32);
      }
      else
         printf("unable to read %s\n", pChip->regMap[i].name);
   }
   return true;
}


/******************************************************************************
 SATFE_Command_read()
******************************************************************************/
bool SATFE_Command_read(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int i;
   SATFE_Register *pReg;
   uint32_t uval32;

   BDBG_ASSERT(pChip);

   if (argc < 2)
      SATFE_PrintDescription1("read", "read [reg...]", "Read 1 or more registers.", "reg = register name or list of register names to read", true);
   else
   {
      for (i = 1; i < argc; i++)
      {
         if (SATFE_LookupRegister(pChip, argv[i], &pReg))
         {
            if (pReg->type == SATFE_RegisterType_eISB)
            {
               SATFE_MUTEX(retCode = BAST_ReadRegister(pChip->hAstChannel[pChip->currChannel], pReg->addr, &uval32));
               if (retCode == BERR_SUCCESS)
                  printf("%s = 0x%08X (%u)\n", argv[i], uval32, uval32);
               else
                  goto unable_to_read;
            }
            else
            {
               unable_to_read:
               printf("unable to read %s\n", argv[i]);
            }
         }
         else
            printf("invalid register: %s\n", argv[i]);
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_write()
******************************************************************************/
bool SATFE_Command_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   SATFE_Register *pReg;
   uint32_t val32;

   BDBG_ASSERT(pChip);

   if (argc != 3)
   {
      SATFE_PrintDescription1("write", "write [reg] [val]", "Write to a register.", "reg = register name", false);
      SATFE_PrintDescription2("val = Register value.  Hexadecimal values must have '0x' prefix.", false);
      return true;
   }
   else
   {
      if (SATFE_LookupRegister(pChip, argv[1], &pReg))
      {
         if (SATFE_GetU32FromString(argv[2], &val32) == false)
            return false;

         if (pReg->type == SATFE_RegisterType_eISB)
         {
            SATFE_MUTEX(retCode = BAST_WriteRegister(pChip->hAstChannel[pChip->currChannel], pReg->addr, &val32));
            if (retCode != BERR_SUCCESS)
               printf("BAST_WriteRegister() error 0x%X\n", retCode);
            else
               return true;
         }
         else
            printf("unable to write %s\n", argv[1]);
      }
      else
         printf("invalid register: %s\n", argv[1]);
   }
   return false;
}


/******************************************************************************
 SATFE_Command_config()
******************************************************************************/
bool SATFE_Command_config(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   SATFE_ConfigParam *pParam;
   bool bReadAll = false;
   bool bRead = true;
   bool bBit = false;
   int  i, n, idx, bit = 0, bitval = 0;
   uint32_t val32, mask;
   uint16_t val16;
   char str[64];
   uint8_t val8, buf[16];

   BDBG_ASSERT(pChip);

   if ((argc == 0) || (argc > 4))
   {
      SATFE_PrintDescription1("config", "config <[param] | [param value] | [param bit bitval]>", "Read/write configuration parameter.", "(no options specified) = display full set of configuration parameters", false);
      SATFE_PrintDescription2("param = configuration parameter name", false);
      SATFE_PrintDescription2("value = new value of the configuration parameter.  If no value is given, then the current value of the specified configuration parameter is returned", true);
      return true;
   }

   if (argc == 1)
   {
      bReadAll = true;
      bRead = true;
   }
   else if (argc == 2)
      bRead = true;
   else if (argc == 3)
      bRead = false;
   else if (argc == 4)
   {
      bRead = false;
      bBit = true;
      bit = atoi(argv[2]);
      bitval = atoi(argv[3]);
      if ((bitval != 0) && (bitval != 1))
      {
         printf("invalid bit value (must be 0 or 1)\n");
         return false;
      }
   }

   if (bReadAll)
   {
      for (i = n = 0; pChip->configParamMap[i].name; i++)
      {
         pParam = &(pChip->configParamMap[i]);
         if (pParam->type == SATFE_ConfigParamType_eAmcScrambling)
         {
            sprintf(str, "%u (index)", pChip->amcScramblingIdx[pChip->currAcqSettings]);
            printf("%22s = %-14s", pParam->name, str);
         }
         else if (pParam->type != SATFE_ConfigParamType_eHidden)
         {
            SATFE_GetConfigParamValueString(pChip, str, pParam);
            printf("%s", str);
         }
         else
            continue;
         if (n & 1)
            printf("\n");
         n++;
      }
      printf("\n");
   }
   else
   {
      if (SATFE_LookupConfigParam(pChip, argv[1], &pParam))
      {
         if (bRead)
         {
            if ((pParam->type == SATFE_ConfigParamType_eDefault) || (pParam->type == SATFE_ConfigParamType_eAmcScrambling))
               strcpy(str, "READ/WRITE");
            else if (pParam->type == SATFE_ConfigParamType_eReadOnly)
               strcpy(str, "READ ONLY");
            else
               strcpy(str, "NONE");
            printf("CONFIGURATION PARAMETER: %s\n", pParam->name);
            printf("ACCESS: %s\n", str);
            if (pParam->type != SATFE_ConfigParamType_eAmcScrambling)
               printf("LENGTH : %d byte%c\n", pParam->len, (pParam->len > 1) ? 's' : ' ');
            if (pParam->desc)
               printf("DESCRIPTION : %s\n", pParam->desc);
            else
               printf("FORMAT: unsigned number\n");

            if (pParam->type == SATFE_ConfigParamType_eAmcScrambling)
               sprintf(str, "%u (index)", pChip->amcScramblingIdx[pChip->currAcqSettings]);
            else
            {
               SATFE_GetConfigParamValueString(pChip, str, pParam);
            }
            printf("CURRENT VALUE: %s\n", str);
         }
         else if (pParam->type == SATFE_ConfigParamType_eAmcScrambling)
         {
            idx = atoi(argv[2]);
            if ((idx < 0) || (idx >= 1016))
            {
               printf("AMC scrambling index out of range\n");
               return false;
            }

            return SATFE_WriteAmcScramblingSeq(pChip, idx);
         }
         else if (pParam->type == SATFE_ConfigParamType_eDefault)
         {
            if (bBit)
            {
               if ((bit < 0) || (bit > ((pParam->len*8)-1)))
               {
                  printf("invalid bit number (must be 0-%d)\n", (pParam->len*8)-1);
                  return false;
               }
               mask = 1 << bit;
               SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pParam->id, buf, pParam->len));
               if (retCode)
               {
                  printf("BAST_ReadConfig() error 0x%X\n", retCode);
                  return false;
               }
               switch (pParam->len)
               {
                  case 1:
                     if (bitval)
                        buf[0] |= (uint8_t)mask;
                     else
                        buf[0] &= (uint8_t)(~mask & 0xFF);
                     break;

                  case 2:
                     val16 = (buf[0] << 8) | buf[1];
                     if (bitval)
                        val16 |= (uint16_t)mask;
                     else
                        val16 &= (uint16_t)(~mask & 0xFFFF);
                     buf[0] = (uint8_t)((val16 >> 8) & 0xFF);
                     buf[1] = (uint8_t)(val16 & 0xFF);
                     break;

                  case 4:
                     val32 = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
                     if (bitval)
                        val32 |= mask;
                     else
                        val32 &= ~mask;
                     buf[0] = (uint8_t)((val32 >> 24) & 0xFF);
                     buf[1] = (uint8_t)((val32 >> 16) & 0xFF);
                     buf[2] = (uint8_t)((val32 >> 8) & 0xFF);
                     buf[3] = (uint8_t)(val32 & 0xFF);
                     break;

                  default:
                     printf("unable to perform bit operations on this configuration parameter\n");
                     return true;
               }

            }
            else
            {
               if (pParam->len == 1)
               {
                  if (SATFE_GetU8FromString(argv[2], &val8))
                     buf[0] = val8;
               }
               else if (pParam->len == 2)
               {
                  if (SATFE_GetU16FromString(argv[2], &val16))
                  {
                     buf[0] = (uint8_t)(val16 >> 8);
                     buf[1] = (uint8_t)(val16 & 0xFF);
                  }
               }
               else if (pParam->len == 4)
               {
                  if (SATFE_GetU32FromString(argv[2], &val32))
                  {
                     buf[0] = (uint8_t)(val32 >> 24);
                     buf[1] = (uint8_t)(val32 >> 16);
                     buf[2] = (uint8_t)(val32 >> 8);
                     buf[3] = (uint8_t)(val32 & 0xFF);
                  }
               }
               else
                  goto cannot_write;
            }

            SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pParam->id, buf, pParam->len));
            if (retCode)
            {
               printf("BAST_WriteConfig() error 0x%X\n", retCode);
               return false;
            }

            if (bBit)
            {
               SATFE_GetConfigParamValueString(pChip, str, pParam);
               printf("updated value is: %s\n", str);
            }
         }
         else
         {
            cannot_write:
            printf("This configuration parameter cannot be written.\n");
            return false;
         }
      }
      else
      {
         printf("invalid configuration parameter\n");
         return false;
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_get_snr_range()
******************************************************************************/
bool SATFE_Command_get_snr_range(SATFE_Chip *pChip, int argc, char **argv)
{
#define SNR_BUFSIZE 70
   BERR_Code retCode = BERR_SUCCESS;
   BAST_ChannelStatus status;
   uint32_t t, min_snr, max_snr, sum_snr, n, nUnlocks, min_snr_time = 0, max_snr_time = 0, lock_time = 0;
   uint32_t snr_buf[SNR_BUFSIZE], i, j, last_t = 0xFFFFFFFF;
   bool bLocked, bNoAcq = false;

   if ((argc < 2) || (argc > 4))
   {
      SATFE_PrintDescription1("get_snr_range", "get_snr_range [-a] | [<mode> <symbol_rate> [freq]]", "Tune to specified frequency and record min/max/ave SNR",  "freq = Specifies RF tuning frequency", true);
      return true;
   }

   if ((argc == 2)&& (!strcmp(argv[1], "-a")))
      bNoAcq = true;

   if (!bNoAcq)
   {
      if (SATFE_Command_ta(pChip, argc, argv) == false)
         return false;
   }

   SATFE_Platform_StartTimer();

   i = 0;
   n = 0;
   nUnlocks = 0;
   bLocked = false;
   min_snr = 0xFFFFFFFF;
   max_snr = 0;
   sum_snr = 0;
   while (SATFE_Platform_GetChar(false) <= 0)
   {
      SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
      SATFE_GOTO_DONE_ON_ERROR("BAST_GetChannelStatus()", retCode);
      t = SATFE_Platform_GetTimerCount();
      if (status.bDemodLocked == false)
      {
         if (bLocked)
            nUnlocks++;
      }
      else if (bLocked == false)
      {
         lock_time = t;
         last_t = 0xFFFFFFFF;
         printf("Locked\n");
         i = 0;
      }
      bLocked = status.bDemodLocked;
      if (!bLocked)
         continue;

      if (status.snrEstimate < min_snr)
      {
         min_snr_time = t;
         min_snr = status.snrEstimate;
      }
      if (status.snrEstimate > max_snr)
      {
         max_snr_time = t;
         max_snr = status.snrEstimate;
      }
      n++;
      sum_snr += status.snrEstimate;

      t = t - lock_time;
      if (!bNoAcq && (i < SNR_BUFSIZE))
      {
#if 0
         if ((t == last_t) && (t < SNR_BUFSIZE))
         {
            if (status.snrEstimate < snr_buf[t])
               snr_buf[t] = status.snrEstimate;
         }
         else
#endif
         if (t >= i)
         {
            snr_buf[i++] = status.snrEstimate;
         }
         last_t = t;
      }
   }

   if (n > 0)
   {
      printf("Locked->NotLocked transitions = %d\n", nUnlocks);
      printf("lock time = %u\n", lock_time);
      printf("min SNR = %.2f (t = %u)\n", (double)min_snr / 256.0, min_snr_time);
      printf("max SNR = %.2f (t = %u)\n", (double)max_snr / 256.0, max_snr_time);
      printf("max-min = %.2f\n", (max_snr - min_snr) / 256.0);
      printf("ave SNR = %.2f\n", (double)sum_snr / ((double)n * 256.0));

      for (j = 0; (!bNoAcq && (j < i) && (j < SNR_BUFSIZE)); j++)
      {
         printf("%d, %d, %.2f\n", j, j+lock_time, (double)snr_buf[j] / 256.0);
      }
   }
   else
      printf("demod could not lock\n");

   done:
   SATFE_Platform_KillTimer();
   return (retCode == BERR_SUCCESS) ? true : false;
}


/******************************************************************************
 SATFE_Command_ta()
******************************************************************************/
bool SATFE_Command_ta(SATFE_Chip *pChip, int argc, char **argv)
{
   uint32_t freq, val32 = 0;
   int i, j;
   float fval;
   char *str[2];

   BDBG_ASSERT(pChip);

   if ((argc == 0) || (argc > 4))
   {
      SATFE_PrintDescription1("ta", "ta <mode> <symbol_rate> <freq>", "Tune to specified frequency and begin acquisition",  "(no option specified) = Display current acquisition parameters.  No acquisition is done.", false);
      SATFE_PrintDescription2("freq = Specifies RF tuning frequency\n", true);
      return true;
   }

   if (argc == 1)
   {
      printf("mode           = %s\n", SATFE_GetModeString(pChip->acqSettings[pChip->currAcqSettings].mode));
      printf("symbol_rate    = %u sym/sec\n", pChip->acqSettings[pChip->currAcqSettings].symbolRate);
      printf("carrier_offset = %d Hz\n", pChip->acqSettings[pChip->currAcqSettings].carrierFreqOffset);
      printf("acq_ctl        = 0x%08X\n", pChip->acqSettings[pChip->currAcqSettings].acq_ctl);
   }
   else if (pChip->commonCmds.ta)
      return pChip->commonCmds.ta(pChip, argc, argv);
   else
   {
      for (i = 1; i < argc; i++)
      {
         if (strstr(argv[i], "dvb") || strstr(argv[i], "dtv") || strstr(argv[i], "ldpc") || strstr(argv[i], "turbo") || strstr(argv[i], "blind"))
         {
            /* set the mode */
            str[0] = "mode";
            str[1] = argv[i];
            if (SATFE_Command_mode(pChip, 2, str) == false)
               return false;
         }
         else
         {
            for (j = 0; j < (int)strlen(argv[i]); j++)
            {
               if (((argv[i][j] < '0') || (argv[i][j] > '9')) && (argv[i][j] != '.'))
               {
                  printf("syntax error\n");
                  return false;
               }
            }

            fval = (float)atof(argv[i]);
            if (fval < 3100)
               val32 = (uint32_t)(fval * 1000000.0);
            if ((val32 >= 1000000) && (val32 <= 45000000))
               pChip->acqSettings[pChip->currAcqSettings].symbolRate = val32;
            else if (SATFE_GetFreqFromString(pChip, argv[i], &freq) == false)
               return false;
         }
      }

      if (SATFE_TuneAcquire(pChip, freq, NULL) != BERR_SUCCESS)
         return false;
   }
   return true;
}


/******************************************************************************
 SATFE_Command_abort()
******************************************************************************/
bool SATFE_Command_abort(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   bool bAbortAll = false;
   uint32_t i;

   BDBG_ASSERT(pChip);

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("abort", "abort <-a>", "Stops channel acquisition/tracking.", "-a = stop all channels", true);
      return true;
   }

   if (pChip->commonCmds.abort)
      return pChip->commonCmds.abort(pChip, argc, argv);

   if (argc == 2)
   {
      if (!strcmp(argv[1], "-a"))
         bAbortAll = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   if (bAbortAll)
   {
      for (i = 0; (retCode == BERR_SUCCESS) & (i < pChip->chip.nDemods); i++)
      {
         SATFE_MUTEX(retCode = BAST_AbortAcq(pChip->hAstChannel[i]));
      }
   }
   else
   {
      SATFE_MUTEX(retCode = BAST_AbortAcq(pChip->hAstChannel[pChip->currChannel]));
   }

   SATFE_RETURN_ERROR("BAST_AbortAcq()", retCode);
}


/******************************************************************************
 SATFE_Command_status()
******************************************************************************/
bool SATFE_Command_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_ChannelStatus status;
   BAST_Mode mode;
   float input_power, snr;
   uint32_t total_channels;
   int i, chan;
   bool bAll = false;

   BDBG_ASSERT(pChip);

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("status", "status <-a>", "Display channel status", "-a = display abridged status for all demods", true);
      return true;
   }

   for (i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "-a"))
         bAll = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   if (pChip->commonCmds.status)
      return pChip->commonCmds.status(pChip, argc, argv);

   BAST_GetTotalChannels(pChip->hAst, &total_channels);
   for (i = 0; i < (int)total_channels; i++)
   {
      if (bAll)
         chan = (int)i;
      else
         chan = pChip->currChannel;

      SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[chan], &status));
      if (retCode)
      {
         printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
         return false;
      }

      mode = status.mode;

      /* calculate snr in dB */
      snr = (float)((float)status.snrEstimate / 256.0);

      if (bAll)
      {
         printf("SDS%d: demod_lock=%d, bert_lock=%d", i, status.bDemodLocked, status.bBertLocked);
         if (status.bDemodLocked)
            printf(", SNR=%.2f\n", (float)snr);
         else
            printf("\n");
      }
      else
      {
         SATFE_Platform_GetInputPower(pChip, status.RFagc, status.IFagc, (uint32_t)status.agf, status.tunerFreq,
                                      &input_power);

         printf("modulation          = %s\n", SATFE_GetModeString(mode));
         printf("symbol_rate         = %u sym/sec\n", (unsigned int)status.symbolRate);
         printf("symbol_rate_error   = %d sym/sec\n", (int)status.symbolRateError);
         printf("carrier_offset      = %d Hz\n", (int)status.carrierOffset);
         printf("carrier_error       = %d Hz\n", (int)status.carrierError);
         printf("tuner_freq          = %u Hz\n", (unsigned int)status.tunerFreq);
         printf("snr                 = %f dB\n", (float)snr);
         printf("power               = %f dBm\n", input_power);
         printf("IFagc,RFagc,AGF     = 0x%08X, 0x%08X, 0x%08X\n", status.IFagc, status.RFagc, status.agf);
         printf("output_bit_rate     = %u bps\n", (unsigned int)status.outputBitrate);
         printf("sample freq         = %u Hz\n", (unsigned int)status.sample_clock);
         printf("tuner LPF cutoff    = %u Hz\n", (unsigned int)status.tunerFilter);
         printf("BER errors          = %u\n", (unsigned int)status.berErrors);
         printf("MPEG frame errors   = %u\n", (unsigned int)status.mpegErrors);
         printf("MPEG frame count    = %u\n", (unsigned int)status.mpegCount);
         printf("Reacquisitions      = %u\n", (unsigned int)status.reacqCount);

         if (BAST_MODE_IS_LDPC(mode))
         {
            printf("fec clock           = %u Hz\n", (unsigned int)status.modeStatus.ldpc.fec_clock);
            printf("total blocks        = %u\n", (unsigned int)status.modeStatus.ldpc.totalBlocks);
            printf("corr blocks         = %u\n", (unsigned int)status.modeStatus.ldpc.corrBlocks);
            printf("bad blocks          = %u\n", (unsigned int)status.modeStatus.ldpc.badBlocks);
            printf("sp inv state        = %s\n", SATFE_IsSpinv(pChip, &status) ? "inverted" : "normal");
            printf("pilot               = %s\n", SATFE_IsPilotOn(pChip, &status) ? "yes" : "no");
         }
         else if (BAST_MODE_IS_TURBO(mode))
         {
            printf("fec clock           = %u Hz\n", (unsigned int)status.modeStatus.turbo.fec_clock);
            printf("total blocks        = %u\n", (unsigned int)status.modeStatus.turbo.totalBlocks);
            printf("corr blocks         = %u\n", (unsigned int)status.modeStatus.turbo.corrBlocks);
            printf("bad blocks          = %u\n", (unsigned int)status.modeStatus.turbo.badBlocks);
            printf("sp inv state        = %s\n", SATFE_IsSpinv(pChip, &status) ? "inverted" : "normal");
         }
         else if (BAST_MODE_IS_LEGACY_QPSK(mode))
         {
            printf("RS corr errors      = %u\n", (unsigned int)status.modeStatus.legacy.rsCorrCount);
            printf("RS uncorr errors    = %u\n", (unsigned int)status.modeStatus.legacy.rsUncorrCount);
            printf("Pre-Vit errors      = %u\n", (unsigned int)status.modeStatus.legacy.preVitErrCount);
            printf("sp inv state        = %s\n", (status.modeStatus.legacy.spinv == 0) ? "normal" : "inverted");
         }

         printf("tuner is            = %s\n", status.bTunerLocked ? "LOCKED" : "not locked");
         printf("internal BERT is    = %s\n", status.bBertLocked ? "LOCKED" : "not locked");
         printf("Demod/FEC is        = %s\n", status.bDemodLocked ? "LOCKED" : "not locked");
         break;
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_ver()
******************************************************************************/
bool SATFE_Command_ver(SATFE_Chip *pChip, int argc, char **argv)
{
   BFEC_VersionInfo versionInfo;
   BERR_Code  retCode;

   BDBG_ASSERT(pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ver", "ver", "Display frontend chip version information", "none", true);
      return true;
   }

   if (pChip->commonCmds.ver)
      return pChip->commonCmds.ver(pChip, argc, argv);

   SATFE_MUTEX(retCode = BAST_GetVersionInfo(pChip->hAst, &versionInfo));
   if (retCode == BERR_SUCCESS)
   {
      printf("Version %d.%d, Build %d\n", versionInfo.majorVersion, versionInfo.minorVersion, versionInfo.buildId);
   }
   else
   {
      uint32_t   bondId;
      uint16_t   chipId;
      uint8_t    fwVer, cfgVer, chipVer;

      SATFE_MUTEX(retCode = BAST_GetApVersion(pChip->hAst, &chipId, &chipVer, &bondId, &fwVer, &cfgVer));
      if (retCode)
      {
         printf("BAST_GetApVersion() error 0x%X\n", retCode);
         return false;
      }

      printf("BCM%04X: chip version 0x%X, FW version %d.%d, bondout=0x%08X\n",
             chipId, chipVer, fwVer, cfgVer, bondId);
   }
   return true;
}


/******************************************************************************
 SATFE_Command_chip()
******************************************************************************/
bool SATFE_Command_chip(SATFE_Chip *pChip, int argc, char **argv)
{
   int i;
   BDBG_ASSERT(pChip);

   if ((argc == 0) || (argc > 2))
   {
      SATFE_PrintDescription1("chip", "chip <idx>", "Select frontend chip to control.", "(no option specified) = Display list of frontend chips on this board.", false);
      SATFE_PrintDescription2("idx = Specifies the index of the chip to select.", true);
      return true;
   }

   if (argc == 1)
   {
      for (i = 0; i < SATFE_NUM_CHIPS; i++)
         printf("%d. %s\n", i, g_pSATFE[i].chip.name);
      printf("current chip idx is %d\n", g_SATFE_curr_chip_idx);
   }
   else
   {
      i = atoi(argv[1]);
      if ((i < 0) || (i >= SATFE_NUM_CHIPS))
         printf("invalid chip index\n");
      else
         g_SATFE_curr_chip_idx = i;
   }

   return true;
}


/******************************************************************************
 SATFE_Command_channel()
******************************************************************************/
bool SATFE_Command_channel(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int i;
   uint32_t numChannels;
   BDBG_ASSERT(pChip);

   if ((argc == 0) || (argc > 2))
   {
      SATFE_PrintDescription1("channel", "channel <idx>", "Select downstream channel to control.", "(no option specified) = Display current channel.", false);
      SATFE_PrintDescription2("idx = Specifies the channel to control.", true);
      return true;
   }

   if ((retCode = BAST_GetTotalChannels(pChip->hAst, &numChannels)) != BERR_SUCCESS)
   {
      printf("BAST_GetTotalChannels() error 0x%X\n", retCode);
      numChannels = pChip->chip.nDemods;
   }

   if (argc == 1)
   {
      printf("Valid channels are: ");
      for (i = 0; i < (int32_t)numChannels; i++)
      {
         if (i)
            printf(", ");
         printf("%d", i);
      }
      printf("\n");
      printf("The current channel is %d.\n", pChip->currChannel);

   }
   else
   {
      i = atoi(argv[1]);
      if ((i < 0) || (i >= (int32_t)numChannels))
      {
         printf("invalid channel index (%d), numChannels=%d\n", i, numChannels);
         return false;
      }
      if (pChip->currChannel != (uint8_t)i)
      {
         pChip->currChannel = (uint8_t)i;
         pChip->currAcqSettings = (i * 2);
      }
   }

   return true;
}


/******************************************************************************
 SATFE_Command_mode()
******************************************************************************/
bool SATFE_Command_mode(SATFE_Chip *pChip, int argc, char **argv)
{
   int i;
   char msg[720];

   BDBG_ASSERT(pChip);

   if (argc == 1)
      printf("mode = %s\n", SATFE_GetModeString(pChip->acqSettings[pChip->currAcqSettings].mode));
   else if (argc == 2)
   {
      for (i = 0; SATFE_ModeList[i].name; i++)
      {
         if (!strcmp(SATFE_ModeList[i].name, argv[1]))
         {
            pChip->acqSettings[pChip->currAcqSettings].mode = SATFE_ModeList[i].mode;
            return true;
         }
      }
      printf("invalid mode\n");
   }
   else
   {
      SATFE_PrintDescription1("mode", "mode <m>", "Specify modulation/code_rate for next acquisition.", "(no option specified) = Display current mode.", false);
      strcpy(msg, "m = Specifies the modulation/code rate.  Valid values are: ");
      for (i = 0; SATFE_ModeList[i].name; i++)
      {
         if (i)
            strcat(msg, ", ");
         strcat(msg, SATFE_ModeList[i].name);
      }
      SATFE_PrintDescription2(msg, true);
   }
   return true;
}

/******************************************************************************
 SATFE_Command_symbol_rate()
******************************************************************************/
bool SATFE_Command_symbol_rate(SATFE_Chip *pChip, int argc, char **argv)
{
   float f;
   uint32_t sps;
   BDBG_ASSERT(pChip);

   if ((argc == 0) || (argc > 2))
   {
      SATFE_PrintDescription1("symbol_rate", "symbol_rate <sps>", "Specify symbol rate for next acquisition.", "(no option specified) = Display current symbol rate.", false);
      SATFE_PrintDescription2("sps = Specifies the symbol rate in either sym/sec or Msym/sec.", true);
      return true;
   }

   if (argc == 1)
   {
      printf("symbol_rate = %d sym/s\n", pChip->acqSettings[pChip->currAcqSettings].symbolRate);
   }
   else
   {
      f = (float)atof(argv[1]);
      if (f < 50)
         sps = (uint32_t)(f * 1000000);
      else
         sps = (uint32_t)f;

      if ((sps < 256000UL) || (sps > 45000000UL))
      {
         printf("symbol rate out of range\n");
         return false;
      }

      pChip->acqSettings[pChip->currAcqSettings].symbolRate = sps;
   }

   return true;
}

/******************************************************************************
 SATFE_Command_acq_ctl()
******************************************************************************/
bool SATFE_Command_acq_ctl(SATFE_Chip *pChip, int argc, char **argv)
{
   int bit, value;
   uint32_t mask;

   BDBG_ASSERT(pChip);

   if ((argc < 2) || (argc > 3))
   {
      SATFE_PrintDescription1("acq_ctl", "acq_ctl <ctl | [bit value]>", "Specify acquisition options.", "(no option specified) = Display current options.", false);
      SATFE_PrintDescription2("ctl = 32-bit control word having follow definition:", false);
      SATFE_PrintDescription2(
         "      bit 0: 0=MPEG, 1=PN\n   "
         "      bit 1: 0=PRBS-23, 1=PRBS-15\n   "
         "      bit 2: nyquist filter rolloff: 0=.35, 1=.20\n   "
         "      bit 3: BERT: 0=disable, 1=enable\n   "
         "      bit 4: PN data: 0=normal, 1=inverted\n   "
         "      bit 5: BERT resync: 0=enabled, 1=disabled\n   "
         "      bit 6: auto reacquisition: 0=enable, 1=disable\n   "
         "      bit 7: reserved\n   "
         "      bit 8: DCII only: 0=combined, 1=split\n   "
         "      bit 9: DCII split mode only: 0=split I, 1=split Q\n   "
         "      bit 10: 0=QPSK, 1=OQPSK\n   "
         "      bit 11: reserved\n   "
         "      bit 12: LDPC pilot scan: 0=disabled, 1=enabled\n   "
         "      bit 14-13: spect inv: 00=IQ_normal, 01=Q_inv, 10=I_inv, 11=IQ_scan\n   "
         "      bit 15: 0=RS enabled, 1=RS disabled\n   "
         "      bit 16: LDPC only: 0=no pilot, 1=pilot\n   "
         "      bit 17: LDPC pilot PLL mode: 0=disabled, 1=enabled\n   "
         "      bit 18: tuner test mode: 0=disabled, 1=enabled\n   "
         "      bit 19: LDPC only: 0=normal FEC frames, 1=short FEC frames\n"
         "      bit 21: signal detect mode: 0=disabled, 1=enabled\n   "
         "      bits 29-19: reserved\n   "
         "      bit 30: 0=normal, 1=bypass tuning\n   "
         "      bit 31: 0=normal, 1=bypass acquisition", false);
      SATFE_PrintDescription2("[bit value] = Set single bit.  bit is 0-31.  value is 0 or 1.", true);

      if (argc == 1)
         printf("acq_ctl = 0x%08X\n", pChip->acqSettings[pChip->currAcqSettings].acq_ctl);

      return true;
   }

   if (argc == 2)
      pChip->acqSettings[pChip->currAcqSettings].acq_ctl = strtoul(argv[1], NULL, 16);
   else
   {
      bit = atoi(argv[1]);
      value = atoi(argv[2]);
      if ((bit < 0) || (bit > 31))
         printf("invalid bit index (must be 0-31)\n");
      else if ((value < 0) || (value > 1))
         printf("invalid value (must be 0 or 1)\n");
      else
      {
         mask = 1 << bit;
         if (value == 0)
            pChip->acqSettings[pChip->currAcqSettings].acq_ctl &= ~mask;
         else
            pChip->acqSettings[pChip->currAcqSettings].acq_ctl |= mask;
      }
   }

   return true;
}

/******************************************************************************
 SATFE_Command_carrier_offset()
******************************************************************************/
bool SATFE_Command_carrier_offset(SATFE_Chip *pChip, int argc, char **argv)
{
   BDBG_ASSERT(pChip);

   if ((argc == 0) || (argc > 2))
   {
      SATFE_PrintDescription1("carrier_offset", "carrier_offset <hz>", "Specify frequency offset for next acquisition.", "(no option specified) = Display current frequency offset.", false);
      SATFE_PrintDescription2("hz = Specifies the frequency offset in Hz.", true);
      return true;
   }

   if (argc == 1)
      printf("carrier_offset = %d Hz\n", pChip->acqSettings[pChip->currAcqSettings].carrierFreqOffset);
   else
      pChip->acqSettings[pChip->currAcqSettings].carrierFreqOffset = atoi(argv[1]);

   return true;
}


/******************************************************************************
 SATFE_Command_monitor()
******************************************************************************/
bool SATFE_Command_monitor(SATFE_Chip *pChip, int argc, char **argv)
{
#define SATFE_MonitorType_eSnr             0x80000000
#define SATFE_MonitorType_eCarrierError    0x80000001
#define SATFE_MonitorType_ePower           0x80000002
#define SATFE_MonitorType_eSymbolRateError 0x80000003
#define SATFE_MonitorType_eReacqs          0x80000004
#define SATFE_MonitorType_eMpegFrameCount  0x80000005
#define SATFE_MonitorType_eMpegFrameErrors 0x80000006
#define SATFE_MonitorType_eBerErrors       0x80000007
#define SATFE_MonitorType_eLock            0x80000008
#define SATFE_MonitorType_eTotalBlocks     0x80000009
#define SATFE_MonitorType_eBadBlocks       0x8000000A
#define SATFE_MonitorType_eCorrBlocks      0x8000000B

   BERR_Code retCode;
   BAST_ChannelStatus status;
   float input_power;
   int i, n = 0;
   uint32_t uval32, *pMonitor = NULL, *pLastRegVal = NULL;
   SATFE_Register *pReg;
   bool bGetStatus = false, bLogRegisterChanges = false, b, bForceNewLine;
   char key;

   BDBG_ASSERT(pChip);

   if (pChip->commonCmds.monitor)
      return pChip->commonCmds.monitor(pChip, argc, argv);

   if (argc < 2)
   {
      SATFE_PrintDescription1("monitor", "monitor <-c> [...]", "Monitor a list of status parameters and/or registers.", "List that includes register names and/or following status parameters:", false);
      SATFE_PrintDescription2("   -c = log all register changes", false);
      SATFE_PrintDescription2("   snr = SNR in dB", false);
      SATFE_PrintDescription2("   carrier_error = carrier freq offset in Hz", false);
      SATFE_PrintDescription2("   power = input power estimate in dBm", false);
      SATFE_PrintDescription2("   symbol_rate_error = symbol rate error in sym/sec", false);
      SATFE_PrintDescription2("   reacqs = number of reacquisitions", false);
      SATFE_PrintDescription2("   mpeg_frame_count = number of MPEG frames transmitted", false);
      SATFE_PrintDescription2("   mpeg_frame_errors = number of MPEG frame errors", false);
      SATFE_PrintDescription2("   ber_errors = BER error count", false);
      SATFE_PrintDescription2("   lock = demod lock status", false);
      SATFE_PrintDescription2("   total_blocks = BCH total blocks", false);
      SATFE_PrintDescription2("   bad_blocks = BCH bad blocks", false);
      SATFE_PrintDescription2("   corr_blocks = BCH corrected blocks", true);
      return true;

   }

   pMonitor = (uint32_t*)BKNI_Malloc((argc - 1) * sizeof(uint32_t));
   pLastRegVal = (uint32_t*)BKNI_Malloc((argc - 1) * sizeof(uint32_t));
   for (i = 1; i < argc; i++)
   {
      b = true;
      if (!strcmp(argv[i], "-c"))
      {
         b = false;
         bLogRegisterChanges = true;
         pMonitor[i-1] = 0;
      }
      else if (!strcmp(argv[i], "snr"))
         pMonitor[i-1] = SATFE_MonitorType_eSnr;
      else if (!strcmp(argv[i], "carrier_error"))
         pMonitor[i-1] = SATFE_MonitorType_eCarrierError;
      else if (!strcmp(argv[i], "power"))
         pMonitor[i-1] = SATFE_MonitorType_ePower;
      else if (!strcmp(argv[i], "symbol_rate_error"))
         pMonitor[i-1] = SATFE_MonitorType_eSymbolRateError;
      else if (!strcmp(argv[i], "reacqs"))
         pMonitor[i-1] = SATFE_MonitorType_eReacqs;
      else if (!strcmp(argv[i], "mpeg_frame_count"))
         pMonitor[i-1] = SATFE_MonitorType_eMpegFrameCount;
      else if (!strcmp(argv[i], "mpeg_frame_errors"))
         pMonitor[i-1] = SATFE_MonitorType_eMpegFrameErrors;
      else if (!strcmp(argv[i], "ber_errors"))
         pMonitor[i-1] = SATFE_MonitorType_eBerErrors;
      else if (!strcmp(argv[i], "lock"))
         pMonitor[i-1] = SATFE_MonitorType_eLock;
      else if (!strcmp(argv[i], "total_blocks"))
         pMonitor[i-1] = SATFE_MonitorType_eTotalBlocks;
      else if (!strcmp(argv[i], "bad_blocks"))
         pMonitor[i-1] = SATFE_MonitorType_eBadBlocks;
      else if (!strcmp(argv[i], "corr_blocks"))
         pMonitor[i-1] = SATFE_MonitorType_eCorrBlocks;
      else if (SATFE_LookupRegister(pChip, argv[i], &pReg))
      {
         if (pReg->type == SATFE_RegisterType_eISB)
         {
            pLastRegVal[i-1] = 0xFFFFFFFF;
            pMonitor[i-1] = pReg->addr;
         }
         else
         {
            printf("%s is not a valid ISB register\n", argv[i]);
            goto done;
         }
      }
      else
      {
         printf("invalid parameter: %s\n", argv[i]);
         goto done;
      }

      if (b)
         n++;

/* printf("pMonitor[%d] = 0x%X\n", i-1, pMonitor[i-1]); */
      if (pMonitor[i-1] & 0x80000000)
         bGetStatus = true;
   }

   printf("press 'r' to reset status, press 'q' to quit\n");
   while (1)
   {
      bForceNewLine = false;

      if ((key = SATFE_Platform_GetChar(false)) > 0)
      {
         /* key was pressed */
         bForceNewLine = true;

         if (key == 'q')
            break;
         else if (key == 'r')
         {
            SATFE_MUTEX(retCode = BAST_ResetStatus(pChip->hAstChannel[pChip->currChannel]));
            if (retCode)
            {
               printf("BAST_ResetStatus() error 0x%X\n", retCode);
               goto done;
            }
         }
      }

      if (bGetStatus)
      {
         SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
         if (retCode)
         {
            printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
            goto done;
         }
      }

      for (i = 0; i < n; i++)
      {
         if (i > 0)
            printf(", ");

         switch (pMonitor[i])
         {
            case SATFE_MonitorType_eSnr:
               printf("snr=%.2f", (float)((float)status.snrEstimate / 256.0));
               break;

            case SATFE_MonitorType_eCarrierError:
               printf("carrier_error=%9d", status.carrierError);
               break;

            case SATFE_MonitorType_ePower:
               SATFE_Platform_GetInputPower(pChip, status.RFagc, status.IFagc, (uint32_t)status.agf, status.tunerFreq, &input_power);
               printf("power=%.2f", input_power);
               break;

            case SATFE_MonitorType_eSymbolRateError:
               printf("symbol_rate_error=%9d", status.symbolRateError);
               break;

            case SATFE_MonitorType_eReacqs:
               printf("reacqs=%u", status.reacqCount);
               break;

            case SATFE_MonitorType_eMpegFrameCount:
               printf("mpeg_frame_count=%u", status.mpegCount);
               break;

            case SATFE_MonitorType_eMpegFrameErrors:
               printf("mpeg_frame_errors=%u", status.mpegErrors);
               break;

            case SATFE_MonitorType_eBerErrors:
               printf("ber_errors=%u", status.berErrors);
               break;

            case SATFE_MonitorType_eLock:
               printf("%s", status.bDemodLocked ? "LOCKED" : "NOT_LOCKED");
               break;

            case SATFE_MonitorType_eTotalBlocks:
               printf("total_blks=%u", status.modeStatus.ldpc.totalBlocks);
               break;

            case SATFE_MonitorType_eBadBlocks:
               printf("bad_blks=%u", status.modeStatus.ldpc.badBlocks);
               break;

            case SATFE_MonitorType_eCorrBlocks:
               printf("corr_blks=%u", status.modeStatus.ldpc.corrBlocks);
               break;

            case 0: /* ignore */
               break;

            default: /* register */
               SATFE_MUTEX(retCode = BAST_ReadRegister(pChip->hAstChannel[pChip->currChannel], pMonitor[i], &uval32));
               if (retCode == BERR_SUCCESS)
               {
                  printf("%s=0x%08X", argv[i+1], uval32);
                  if (pLastRegVal[i] != uval32)
                  {
                     pLastRegVal[i] = uval32;
                     bForceNewLine = true;
                  }
               }
               else
               {
                  printf("BAST_ReadRegister() error 0x%X\n", retCode);
                  goto done;
               }
               break;
         }
      }

      if ((bGetStatus && !status.bDemodLocked) || bForceNewLine)
         printf("\n");
      else
         printf("\r");

      SATFE_OnIdle();
   }

   done:
   printf("\n");
   BKNI_Free(pMonitor);
   BKNI_Free(pLastRegVal);
   return true;
}


/******************************************************************************
 SATFE_Command_reset_status()
******************************************************************************/
bool SATFE_Command_reset_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;

   BSTD_UNUSED(argv);

   BDBG_ASSERT(pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("reset_status", "reset_status", "Reset status counters.", "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BAST_ResetStatus(pChip->hAstChannel[pChip->currChannel]));
   SATFE_RETURN_ERROR("BAST_ResetStatus()", retCode);
}


/******************************************************************************
 SATFE_Command_diseqc_reset()
******************************************************************************/
bool SATFE_Command_diseqc_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;

   BDBG_ASSERT(pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("diseqc_reset", "diseqc_reset", "Reset and initialize the DISEQC block.", "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BAST_ResetDiseqc(pChip->hAstChannel[pChip->currChannel], 0));
   if (retCode)
   {
      printf("BAST_ResetDiseqc() error 0x%X\n", retCode);
      return false;
   }

   if (pChip->commonCmds.diseqc_reset)
      return pChip->commonCmds.diseqc_reset(pChip, argc, argv);

   return true;
}

/******************************************************************************
 bool SATFE_Command_config_gpio()
******************************************************************************/
bool SATFE_Command_config_gpio(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t write_mask, read_mask;

   if (argc != 3)
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    configures the GPIO pins\n");
      printf("SYNTAX:     config_gpio [write_mask_hex16] [read_mask_hex16]\n");
      printf("PARAMETERS: write_mask_hex16 = bit mask specifying which GPIO pins are output\n");
      printf("            read_mask_hex16 = bit mask specifying which GPIO pins are input\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   write_mask = strtoul(argv[1], NULL, 16);
   read_mask = strtoul(argv[2], NULL, 16);
   SATFE_MUTEX(retCode = BAST_ConfigGpio(pChip->hAst, write_mask, read_mask));
   SATFE_RETURN_ERROR("BAST_ConfigGpio()", retCode);
}


/******************************************************************************
 bool SATFE_Command_set_gpio()
******************************************************************************/
bool SATFE_Command_set_gpio(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t write_mask, data_mask;

   if (argc != 3)
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    sets the state of GPIO pin(s)\n");
      printf("SYNTAX:     set_gpio [write_mask] [data_mask]\n");
      printf("PARAMETERS: write_mask = specifies which GPIO pins to set\n");
      printf("            data_mask = state of the pins to set\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   write_mask = strtoul(argv[1], NULL, 16);
   data_mask = strtoul(argv[2], NULL, 16);
   SATFE_MUTEX(retCode = BAST_SetGpio(pChip->hAst, write_mask, data_mask));
   SATFE_RETURN_ERROR("BAST_SetGpio()", retCode);
}


/******************************************************************************
 bool SATFE_Command_get_gpio()
******************************************************************************/
bool SATFE_Command_get_gpio(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t read_mask, data_mask, i;

   if (argc != 2)
   {
      printf("------------------------------------------------------------------------------\n");
      printf("PURPOSE:    gets the state of GPIO pin(s)\n");
      printf("SYNTAX:     get_gpio [read_mask]\n");
      printf("PARAMETERS: write_mask = selects which GPIO pins to read\n");
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   read_mask = strtoul(argv[1], NULL, 16);
   SATFE_MUTEX(retCode = BAST_GetGpio(pChip->hAst, read_mask, &data_mask));
   if (retCode == BERR_SUCCESS)
   {
      for (i = 0; i < 16; i++)
      {
         if (read_mask & (1 << i))
            printf("GPIO_%d = %d\n", i, ((data_mask & (1 << i)) ? 1 : 0));
      }
   }
   SATFE_RETURN_ERROR("BAST_GetGpio()", retCode);
}

/******************************************************************************
 SATFE_Command_vtop()
******************************************************************************/
bool SATFE_Command_vtop(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;

   BDBG_ASSERT(pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("vtop", "vtop", "Sets the LNB voltage level to 18V.", "none", true);
      return true;
   }

   if (pChip->commonCmds.vtop)
      return pChip->commonCmds.vtop(pChip, argc, argv);

   SATFE_MUTEX(retCode = BAST_SetDiseqcVoltage(pChip->hAstChannel[pChip->currChannel], true));
   SATFE_RETURN_ERROR("BAST_SetDiseqcVoltage()", retCode);
}


/******************************************************************************
 SATFE_Command_vbot()
******************************************************************************/
bool SATFE_Command_vbot(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;

   BDBG_ASSERT(pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("vbot", "vbot", "Sets the LNB voltage level to 13V.", "none", true);
      return true;
   }

   if (pChip->commonCmds.vbot)
      return pChip->commonCmds.vbot(pChip, argc, argv);

   SATFE_MUTEX(retCode = BAST_SetDiseqcVoltage(pChip->hAstChannel[pChip->currChannel], false));
   SATFE_RETURN_ERROR("BAST_SetDiseqcVoltage()", retCode);
}


/******************************************************************************
 SATFE_Command_tone()
******************************************************************************/
bool SATFE_Command_tone(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bTone;

   BDBG_ASSERT(pChip);

   if ((argc < 1) || (argc > 2))
   {
      SATFE_PrintDescription1("tone", "tone <on | off>", "Generate/remove or detect 22KHz tone", "on = turn on 22KHz tone", false);
      SATFE_PrintDescription2("off = turn off 22KHz tone", false);
      SATFE_PrintDescription2("(no option specified) = determine if tone is present", true);
      return true;
   }

   if (pChip->commonCmds.tone)
      return pChip->commonCmds.tone(pChip, argc, argv);

   if (argc == 1)
   {
      /* detect tone */
      SATFE_MUTEX(retCode = BAST_GetDiseqcTone(pChip->hAstChannel[pChip->currChannel], &bTone));
      if (retCode == BERR_SUCCESS)
         printf("22KHz tone is %s\n", bTone ? "present" : "absent");
   }
   else
   {
      if (!strcmp(argv[1], "on"))
         bTone = true;
      else if (!strcmp(argv[1], "off"))
         bTone = false;
      else
      {
         printf("syntax error\n");
         return false;
      }

      /* set tone */
      SATFE_MUTEX(retCode = BAST_SetDiseqcTone(pChip->hAstChannel[pChip->currChannel], bTone));
   }

   SATFE_RETURN_ERROR("BAST_SetDiseqcTone()", retCode);
}


/******************************************************************************
 SATFE_Command_acw()
******************************************************************************/
bool SATFE_Command_acw(SATFE_Chip *pChip, int argc, char **argv)
{
   BKNI_EventHandle diseqcEvent;
   BAST_DiseqcStatus status;
   BERR_Code retCode;
   uint32_t i;
   uint8_t acw;

   if (argc != 2)
   {
      SATFE_PrintDescription1("acw", "acw [hex_byte]", "Transmit auto-control word.", "hex_byte = 8-bit auto control word to send", true);
      return true;
   }

   acw = (uint8_t)strtoul(argv[1], NULL, 16);

   /* get the diseqc event and clear it */
   SATFE_MUTEX(retCode = BAST_GetDiseqcEventHandle(pChip->hAstChannel[pChip->currChannel], &diseqcEvent));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_GetDiseqcEventHandle() error 0x%X\n", retCode);
      goto done;
   }
   BKNI_WaitForEvent(diseqcEvent, 0);

   SATFE_MUTEX(retCode = BAST_SendACW(pChip->hAstChannel[pChip->currChannel], acw));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_SendACW() error 0x%X\n", retCode);
      goto done;
   }

   /* wait for transaction done event */
#if defined(DIAGS) || defined(WIN32)
   for (i = 0; i < 1000; i++)
   {
      SATFE_OnIdle();
      BKNI_Sleep(1);
      retCode = BKNI_WaitForEvent(diseqcEvent, 0);
      if (retCode == BERR_SUCCESS)
         break;
   }
#else
   retCode = BKNI_WaitForEvent(diseqcEvent, 1000);
#endif

   if (retCode == BERR_SUCCESS)
   {
      /* get the transaction status */
      SATFE_MUTEX(retCode = BAST_GetDiseqcStatus(pChip->hAstChannel[pChip->currChannel], &status));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetDiseqcStatus() error 0x%X\n", retCode);
         goto done;
      }
      switch (status.status)
      {
         case BAST_DiseqcSendStatus_eSuccess:
            printf("OK\n");
            break;

         case BAST_DiseqcSendStatus_eAcwTimeout:
            printf("ERROR: ACW timeout\n");
            break;

         default:
            printf("ERROR: unknown ACW error\n");
            break;
      }
   }
   else if (retCode == BERR_TIMEOUT)
      printf("Diseqc ACW transaction timeout!\n");
   else
      printf("BKNI_WaitForEvent() error 0x%X\n", retCode);

   done:
   return true;
}


/******************************************************************************
 SATFE_Command_send()
******************************************************************************/
bool SATFE_Command_send(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_DiseqcStatus status;
   BKNI_EventHandle diseqcEvent;
   uint8_t sendBufLen, sendBuf[128], *pSendBuf;
   uint32_t i;

   BDBG_ASSERT(pChip);

   if (argc == 0)
   {
      SATFE_PrintDescription1("send", "send <cmd_hex_bytes ...>", "Send DISEQC command.", "(no data specified) = put in receive-only mode.", false);
      SATFE_PrintDescription2("cmd_hex_bytes = 1 or more hex bytes (space delimited) to transmit", true);
      return true;
   }

   if (pChip->commonCmds.send)
      return pChip->commonCmds.send(pChip, argc, argv);

   if (argc > 1)
   {
      for (i = 1; i < (uint32_t)argc; i++)
         sendBuf[i - 1] = (uint8_t)strtoul(argv[i], NULL, 16);
      pSendBuf = sendBuf;
      sendBufLen = argc - 1;
   }
   else
   {
      pSendBuf = NULL;
      sendBufLen = 0;
   }

   /* get the diseqc event and clear it */
   SATFE_MUTEX(retCode = BAST_GetDiseqcEventHandle(pChip->hAstChannel[pChip->currChannel], &diseqcEvent));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_GetDiseqcEventHandle() error 0x%X\n", retCode);
      goto done;
   }
   BKNI_WaitForEvent(diseqcEvent, 0);

   SATFE_MUTEX(retCode = BAST_SendDiseqcCommand(pChip->hAstChannel[pChip->currChannel], pSendBuf, sendBufLen));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_SendDiseqcCommand() error 0x%X\n", retCode);
      goto done;
   }

   /* wait for transaction done event */
   if (sendBufLen)
   {
#if defined(DIAGS) || defined(WIN32)
      for (i = 0; i < 3000; i++)
      {
         SATFE_OnIdle();
         BKNI_Sleep(1);
         retCode = BKNI_WaitForEvent(diseqcEvent, 0);
         if (retCode == BERR_SUCCESS)
            break;
      }
#else
   retCode = BKNI_WaitForEvent(diseqcEvent, 3000);   /* increased txn timeout for 128 bytes */
#endif
   }
   else
   {
      while (1)
      {
         /* abort rx-only mode on keypress */
         if (SATFE_Platform_GetChar(false) > 0)
         {
            SATFE_MUTEX(BAST_ResetDiseqc(pChip->hAstChannel[pChip->currChannel], 0));
            break;
         }
#if defined(DIAGS) || defined(WIN32)
         for (i = 0; i < 100; i++)
         {
            SATFE_OnIdle();
            BKNI_Sleep(1);
            retCode = BKNI_WaitForEvent(diseqcEvent, 0);
            if (retCode == BERR_SUCCESS)
               break;
         }
#else
         retCode = BKNI_WaitForEvent(diseqcEvent, 100); /* poll for event */
#endif
       if (retCode == BERR_SUCCESS)
            break;
      }
   }

   if (retCode == BERR_SUCCESS)
   {
      /* get the transaction status and any reply bytes */
      SATFE_MUTEX(retCode = BAST_GetDiseqcStatus(pChip->hAstChannel[pChip->currChannel], &status));
      switch (status.status)
      {
         case BAST_DiseqcSendStatus_eSuccess:
            if (!status.bReplyExpected)
               printf("OK\n");
            if (status.nReplyBytes > 0)
            {
               printf("Data received: ");
               for (i = 0; i < status.nReplyBytes; i++)
                  printf("0x%02X ", status.replyBuffer[i]);
               printf("\n");
            }
            else if (status.bReplyExpected)
               printf("No data received\n");
            break;

         case BAST_DiseqcSendStatus_eRxOverflow:
            printf("ERROR: DISEQC rx overflow\n");
            break;

         case BAST_DiseqcSendStatus_eRxReplyTimeout:
            printf("ERROR: DISEQC rx reply timeout\n");
            break;

         case BAST_DiseqcSendStatus_eRxParityError:
            printf("ERROR: DISEQC rx parity error\n");
            break;

         case BAST_DiseqcSendStatus_eBusy:
            printf("ERROR: busy\n");
            break;

         default:
            printf("ERROR: unknown DISEQC error\n");
            break;
      }
   }
   else if (retCode == BERR_TIMEOUT)
      printf("Diseqc transaction timeout!\n");
   else
      printf("BKNI_WaitForEvent() error 0x%X\n", retCode);

   done:
   return true;
}

/******************************************************************************
 SATFE_Command_test_send()
******************************************************************************/
bool SATFE_Command_test_send(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_DiseqcStatus status;
   BKNI_EventHandle diseqcEvent;
   uint8_t sendBufLen, sendBuf[128], *pSendBuf;
   uint32_t i, rand_delay;
   char key;

   BDBG_ASSERT(pChip);

   if (argc == 0)
   {
      SATFE_PrintDescription1("test_send", "test_send <cmd_hex_bytes ...>", "Send DISEQC command repeatedly with random time in between.", "cmd_hex_bytes = 1 or more hex bytes (space delimited) to transmit", true);
      return true;
   }

   if (pChip->commonCmds.send)
      return pChip->commonCmds.send(pChip, argc, argv);

   if (argc > 1)
   {
      for (i = 1; i < (uint32_t)argc; i++)
         sendBuf[i - 1] = (uint8_t)strtoul(argv[i], NULL, 16);
      pSendBuf = sendBuf;
      sendBufLen = argc - 1;
   }
   else
   {
      printf("no command bytes specified!\n");
      return false;
   }

   /* get the diseqc event and clear it */
   SATFE_MUTEX(retCode = BAST_GetDiseqcEventHandle(pChip->hAstChannel[pChip->currChannel], &diseqcEvent));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_GetDiseqcEventHandle() error 0x%X\n", retCode);
      goto done;
   }
   BKNI_WaitForEvent(diseqcEvent, 0);

   while (1)
   {
      if ((key = SATFE_Platform_GetChar(false)) > 0)
         break;

      rand_delay = 500 + rand() % 1000;
      BKNI_Sleep(rand_delay);

      SATFE_MUTEX(retCode = BAST_SendDiseqcCommand(pChip->hAstChannel[pChip->currChannel], pSendBuf, sendBufLen));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_SendDiseqcCommand() error 0x%X\n", retCode);
         goto done;
      }

      /* wait for transaction done event */
#if defined(DIAGS) || defined(WIN32)
      for (i = 0; i < 3000; i++)
      {
         SATFE_OnIdle();
         BKNI_Sleep(1);
         retCode = BKNI_WaitForEvent(diseqcEvent, 0);
         if (retCode == BERR_SUCCESS)
            break;
      }
#else
      retCode = BKNI_WaitForEvent(diseqcEvent, 3000);   /* increased txn timeout for 128 bytes */
#endif

      if (retCode == BERR_SUCCESS)
      {
         /* get the transaction status and any reply bytes */
         SATFE_MUTEX(retCode = BAST_GetDiseqcStatus(pChip->hAstChannel[pChip->currChannel], &status));
         switch (status.status)
         {
            case BAST_DiseqcSendStatus_eSuccess:
               if (!status.bReplyExpected)
                  printf("OK\n");
               if (status.nReplyBytes > 0)
               {
                  printf("Data received: ");
                  for (i = 0; i < status.nReplyBytes; i++)
                     printf("0x%02X ", status.replyBuffer[i]);
                  printf("\n");
               }
               else if (status.bReplyExpected)
                  printf("No data received\n");
               break;

            case BAST_DiseqcSendStatus_eRxOverflow:
               printf("ERROR: DISEQC rx overflow\n");
               break;

            case BAST_DiseqcSendStatus_eRxReplyTimeout:
               printf("ERROR: DISEQC rx reply timeout\n");
               break;

            case BAST_DiseqcSendStatus_eRxParityError:
               printf("ERROR: DISEQC rx parity error\n");
               break;

            case BAST_DiseqcSendStatus_eBusy:
               printf("ERROR: busy\n");
               break;

            default:
               printf("ERROR: unknown DISEQC error\n");
               break;
         }
      }
      else if (retCode == BERR_TIMEOUT)
         printf("Diseqc transaction timeout!\n");
      else
         printf("BKNI_WaitForEvent() error 0x%X\n", retCode);
   }

   done:
   return true;
}


/******************************************************************************
 SATFE_Command_repeat()
******************************************************************************/
bool SATFE_Command_repeat(struct SATFE_Chip *pChip, int argc, char **argv)
{
   int i, n, delay = 0, my_argc;
   char cmdline[64], *my_argv[64];

   BDBG_ASSERT(pChip);

   if ((argc != 2) && (argc != 4))
   {
      syntax_error:
      SATFE_PrintDescription1("repeat", "repeat <-d delay_ms> [n]", "Repeat the next command n times.", "n = number of times to execute command", false);
      SATFE_PrintDescription2("delay_ms = number of milliseconds to delay between each iteration", true);
      return true;
   }
   if (argc == 2)
      n = atoi(argv[1]);
   else
   {
      if (strcmp(argv[1], "-d"))
      {
         printf("syntax error\n");
         goto syntax_error;
      }
      delay = atoi(argv[2]);
      n = atoi(argv[3]);
   }

   printf("Enter command: ");
   SATFE_GetString(cmdline);

   /* make every character lowercase */
   for (i = 0; i < (int)strlen(cmdline); i++)
      cmdline[i] = tolower(cmdline[i]);

   my_argv[0] = strtok(cmdline, (const char *)" \t\n");
   if (!my_argv[0])
      goto done;

   /* get the rest of the tokens */
   for (my_argc = 1; ; my_argc++)
   {
      my_argv[my_argc] = strtok((char*)NULL, (const char *)" \t\n");
      if (!my_argv[my_argc])
         break;
   }

   for (i = 0; (SATFE_Platform_GetChar(false) <= 0) && (i < n); i++)
   {
      SATFE_ProcessCommand(SATFE_GetCurrChip(), my_argc, my_argv);
      SATFE_OnIdle();
      if (delay)
         BKNI_Sleep(delay);
   }

   done:
   return true;
}


/******************************************************************************
 SATFE_Command_test_acq()
******************************************************************************/
bool SATFE_Command_test_acq(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BAST_AcqSettings bogus_acq_settings, acq_settings;
   BAST_ChannelStatus status;
   int i;
   uint32_t tuner_freq[2] = {0, 0};
   bool bTuneAway = false;
   bool bAbortOnNotLocked = false;
   bool bLockTimeoutOverride = false;
   bool bLockStableTimeOverride = false;
   bool bSwitch = false;
   uint32_t rnd_start_time = 0;
   uint32_t rnd_sym_offset = 0;
   uint32_t rnd_freq_offset = 0;
   int curr_freq_idx, max_acq_count = -1;
   uint32_t lockTimeout = 5000, lockStableTime = 2000;
   uint32_t acq_time, min_acq_time, max_acq_time, sum_acq_time, attempts, success, reacq_count[6];
   int32_t freq_offset = 0, sym_offset = 0;
   uint32_t bogus_freq;
   bool bAborted, bLocked;
   char str[16];

   BDBG_ASSERT(pChip);

   if (argc < 2)
   {
      SATFE_PrintDescription1("test_acq", "test_acq <options> [tuner_freq]", "Test for acquisition probability and acquisition time.", "<-c [n]> = Specifies number of acquisitions in the test.  Default is to continually acquire until key is pressed.", false);
      SATFE_PrintDescription2("<-w> = Tune away and initiate bogus acquisition in between each 'real' acquisition", false);
      SATFE_PrintDescription2("<-u> = Stop test when acquisition fails", false);
      SATFE_PrintDescription2("<-t timeout> = Specifies the lock timeout in msecs", false);
      SATFE_PrintDescription2("<-l lock_stable_time> = Specifies the number of msecs of continuous lock before declaring lock", false);
      SATFE_PrintDescription2("<-r rnd_start_time> = Wait some random time (up to rnd_start_time msecs) before starting next acquisition", false);
      SATFE_PrintDescription2("<-b rnd_sym_offset> = Vary symbol rate offset by up to rnd_sym_offset sym/sec", false);
      SATFE_PrintDescription2("<-f rnd_freq_offset> = Vary carrier frequency offset by up to rnd_freq_offset Hz", false);
      SATFE_PrintDescription2("tuner_freq = Tuner frequency in MHz or Hz (required input parameter)", false);

      SATFE_PrintDescription1("test_acq", "test_acq -s <options>", "Test for acquisition probability and acquisition time when alternating between 2 sets of acquisition parameters.", "-s = alternately acquire 2 sets of acquisition parameters (use 'switch' to configure each set of parameters).", false);;
      SATFE_PrintDescription2("<-c [n]> = Specifies number of acquisitions in the test.  Default is to continually acquire until key is pressed.", false);
      SATFE_PrintDescription2("<-w> = Tune away and initiate bogus acquisition in between each 'real' acquisition", false);
      SATFE_PrintDescription2("<-u> = Stop test when acquisition fails", false);
      SATFE_PrintDescription2("<-s> = alternately acquire 2 sets of acquisition parameters", false);
      SATFE_PrintDescription2("<-t timeout> = Specifies the lock timeout in msecs", false);
      SATFE_PrintDescription2("<-l lock_stable_time> = Specifies the number of msecs of continuous lock before declaring lock", false);
      SATFE_PrintDescription2("<-r rnd_start_time> = Wait some random time (up to rnd_start_time msecs) before starting next acquisition", false);
      SATFE_PrintDescription2("<-b rnd_sym_offset> = Vary symbol rate offset by up to rnd_sym_offset sym/sec", false);
      SATFE_PrintDescription2("<-f rnd_freq_offset> = Vary carrier frequency offset by up to rnd_freq_offset Hz", true);
      return true;
   }

   for (i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "-u"))
         bAbortOnNotLocked = true;
      else if (!strcmp(argv[i], "-w"))
         bTuneAway = true;
      else if (!strcmp(argv[i], "-s"))
         bSwitch = true;
      else if (!strcmp(argv[i], "-c"))
      {
         if (argc <= (i + 2))
            goto syntax_error;
         i++;
         max_acq_count = atoi(argv[i]);
      }
      else if (!strcmp(argv[i], "-t"))
      {
         if (argc <= (i + 2))
            goto syntax_error;
         i++;
         bLockTimeoutOverride = true;
         lockTimeout = strtoul(argv[i], NULL, 10);
      }
      else if (!strcmp(argv[i], "-l"))
      {
         if (argc <= (i + 2))
            goto syntax_error;
         i++;
         bLockStableTimeOverride = true;
         lockStableTime = strtoul(argv[i], NULL, 10);
      }
      else if (!strcmp(argv[i], "-r"))
      {
         if (argc <= (i + 2))
            goto syntax_error;
         i++;
         rnd_start_time = strtoul(argv[i], NULL, 10);
      }
      else if (!strcmp(argv[i], "-b"))
      {
         if (argc <= (i + 2))
            goto syntax_error;
         i++;
         rnd_sym_offset = strtoul(argv[i], NULL, 10);
      }
      else if (!strcmp(argv[i], "-f"))
      {
         if (argc <= (i + 2))
            goto syntax_error;
         i++;
         rnd_freq_offset = strtoul(argv[i], NULL, 10);
      }
      else if (!bSwitch && ((i+1) == argc))
      {
         if (SATFE_GetFreqFromString(pChip, argv[i], &tuner_freq[0]) == false)
         {
            printf("invalid frequency\n");
            return false;
         }
      }
      else
      {
         syntax_error:
         printf("syntax error\n");
         return false;
      }
   }

   if (bSwitch)
   {
      for (curr_freq_idx = 0; curr_freq_idx < 2; curr_freq_idx++)
      {
         retry:
         printf("Enter tuner freq for %u sym/sec %s: ", pChip->acqSettings[curr_freq_idx].symbolRate, SATFE_GetModeString(pChip->acqSettings[curr_freq_idx].mode));
         SATFE_GetString(str);
         if (SATFE_GetFreqFromString(pChip, str, &tuner_freq[curr_freq_idx]) == false)
            goto retry;
      }
   }
   else if (tuner_freq[0] == 0)
   {
      printf("no tuner_freq specified\n");
      return true;
   }

   curr_freq_idx = 0;

   /* initialize stats */
   attempts = success = max_acq_time = sum_acq_time = 0;
   min_acq_time = 0xFFFFFFFF;
   for (i = 0; i < 6; i++)
      reacq_count[i] = 0;

   printf("Press <Enter> to quit\n");
   while (SATFE_Platform_GetChar(false) <= 0)
   {
      if (max_acq_count >= 0)
      {
         if (attempts >= (uint32_t)max_acq_count)
            break;
      }

      if (!bLockTimeoutOverride)
      {
         /* set lockTimeout */
         if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 2000000UL)
            lockTimeout = 5000;
         else if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 1000000)
            lockTimeout = 10000;
         else
            lockTimeout = 15000;
      }

      if (!bLockStableTimeOverride)
      {
         /* set lockStableTime */
         if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 4000000UL)
            lockStableTime = 1000;
         else if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 2000000UL)
            lockStableTime = 2000;
         else if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 1000000UL)
            lockStableTime = 3000;
         else
            lockStableTime = 5000;
      }

      if (rnd_freq_offset)
      {
         /* set freq_offset */
         freq_offset = (SATFE_Platform_Rand() % (rnd_freq_offset << 1)) - rnd_freq_offset;
      }

      if (rnd_sym_offset)
      {
         /* set sym_offset */
         sym_offset = (SATFE_Platform_Rand() % (rnd_sym_offset << 1)) - rnd_sym_offset;
      }

      if (bTuneAway)
      {
         /* tune away */
         bogus_freq = ((SATFE_Platform_Rand() % (2140-960)) + 960) * 1000000;
         if (BAST_MODE_IS_LEGACY_QPSK(pChip->acqSettings[pChip->currAcqSettings].mode))
            bogus_acq_settings.mode = BAST_Mode_eLdpc_scan;
         else
            bogus_acq_settings.mode = BAST_Mode_eDvb_scan;
         bogus_acq_settings.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
         bogus_acq_settings.carrierFreqOffset = 0;
         bogus_acq_settings.symbolRate = 4123456;
         SATFE_TuneAcquire(pChip, bogus_freq, &bogus_acq_settings);

         if (SATFE_WaitForKeyPress(100) > 0)
            break;
      }

      if (rnd_start_time)
      {
         if (SATFE_WaitForKeyPress(SATFE_Platform_Rand() % rnd_start_time) > 0)
            break;
      }

      acq_settings = pChip->acqSettings[pChip->currAcqSettings];
      acq_settings.symbolRate += sym_offset;
      printf("Acquiring %s %d sym/s @ %u Hz...\n", SATFE_GetModeString(acq_settings.mode), acq_settings.symbolRate, tuner_freq[curr_freq_idx] + freq_offset);
      SATFE_TimedAcquire(pChip, tuner_freq[curr_freq_idx] + freq_offset, &acq_settings, lockTimeout, lockStableTime, &bAborted, &bLocked, &status, &acq_time);
      if (bAborted)
         break;
      attempts++;

      if (bLocked)
      {
         success++;
         sum_acq_time += acq_time;
         if (acq_time < min_acq_time)
            min_acq_time = acq_time;
         if (acq_time > max_acq_time)
            max_acq_time = acq_time;
         if (status.reacqCount >= 5)
            reacq_count[5]++;
         else
            reacq_count[status.reacqCount]++;
         printf("%u/%u: LOCKED in %d msecs, %d reacqs, %.2f dB SNR\n", success, attempts, acq_time, status.reacqCount, (float)status.snrEstimate / 256.0);
      }
      else
      {
         printf("%u/%u: NOT LOCKED\n", success, attempts);
      }
      if (bSwitch)
      {
         curr_freq_idx ^= 1;
         pChip->currAcqSettings ^= 1;
      }
   }

   /* display statistics */
   if (attempts > 0)
   {
      printf("\n%u locked / %u total acquisitions (%.2f%% success rate)\n", success, attempts, (100.0 * (float)success) / (float)attempts);
      if (success > 0)
      {
         printf("min/max/ave acquisition time = %u/%u/%.1f msecs\n", min_acq_time, max_acq_time, (float)sum_acq_time / (float)success);
         for (i = 0; i <= 5; i++)
         {
            if (i < 5)
               printf("%4d", i);
            else
               printf(">= 5");
            printf(" reacqs = %d\n", reacq_count[i]);
         }
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_switch()
******************************************************************************/
bool SATFE_Command_switch(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BDBG_ASSERT(pChip);

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("switch", "switch", "Switch between 2 sets of acquistion input parameters.", "none", true);
      return true;
   }

   pChip->currAcqSettings ^= 1;

   printf("mode               = %s\n", SATFE_GetModeString(pChip->acqSettings[pChip->currAcqSettings].mode));
   printf("symbol_rate        = %u sym/sec\n", pChip->acqSettings[pChip->currAcqSettings].symbolRate);
   printf("carrier_offset     = %d Hz\n", pChip->acqSettings[pChip->currAcqSettings].carrierFreqOffset);
   printf("acq_ctl            = 0x%08X\n", pChip->acqSettings[pChip->currAcqSettings].acq_ctl);
   printf("amc_scrambling_seq = %d\n", pChip->amcScramblingIdx[pChip->currAcqSettings]);

   if (pChip->amcScramblingIdx[0] != pChip->amcScramblingIdx[1])
   {
      /* reprogram the scrambling sequence */
      return SATFE_WriteAmcScramblingSeq(pChip, pChip->amcScramblingIdx[pChip->currAcqSettings]);
   }
   return true;
}


/******************************************************************************
 SATFE_Command_ber()
******************************************************************************/
bool SATFE_Command_ber(SATFE_Chip *pChip, int argc, char **argv)
{
   double ber;
   BERR_Code retCode = BERR_SUCCESS;
   BAST_ChannelStatus status;
   uint32_t elapsed_time_ms;
   char input_char;

   BSTD_UNUSED(argv);

   BDBG_ASSERT(pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ber", "ber", "Monitor the BER.  You must first manually lock the internal BERT before running this command.", "none", true);
      return true;
   }

   printf("\nPress 'enter' to reset the BERT.  Press any other key to quit test.\n");
   SATFE_Platform_StartTimer();
   while (1)
   {
      if ((input_char = SATFE_Platform_GetChar(false)) > 0)
      {
         if ((input_char == 10) || (input_char == 0x0D))
         {
            /* reset the BERT */
            printf("\nBERT reset\n");
            SATFE_MUTEX(retCode = BAST_ResetStatus(pChip->hAstChannel[pChip->currChannel]));
            if (retCode)
            {
               printf("BAST_ResetStatus() error 0x%X\n", retCode);
               break;
            }
            SATFE_Platform_StartTimer();
            printf("\nBERT reset\n");
         }
         else
         {
            printf("\naborting test...\n");
            break;
         }
      }

      /* display the BER */
      SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
      if (retCode)
      {
         printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
         break;
      }

      elapsed_time_ms = SATFE_Platform_GetTimerCount();
      if (status.bBertLocked && status.bDemodLocked)
      {
         ber = ((double)status.berErrors * 1000.0) / (double)((double)elapsed_time_ms * (double)status.outputBitrate);
         if (!(BAST_MODE_IS_DTV(status.mode)))
         {
            /* adjust for DVB-S, DVB-S2, and turbo */
            ber *= (188.0 / 187.0);
         }
         printf("%.3f s: BERT locked, %d errs, BER=%e \t\r", (double)(elapsed_time_ms / 1000.0), status.berErrors, ber);
      }
      else
         printf("\n%.3f s: demod %s, BERT %s\n", (double)(elapsed_time_ms / 1000.0), status.bDemodLocked ? "locked" : "not locked",
                status.bBertLocked ? "locked" : "not locked");

      SATFE_OnIdle();
   }

   SATFE_Platform_KillTimer();
   return true;
}


/******************************************************************************
 SATFE_Command_dft_test()
******************************************************************************/
bool SATFE_Command_dft_test(struct SATFE_Chip *pChip, int argc, char **argv)
{
   typedef struct {
      int32_t  offset;
      uint32_t count;
   } hist_t;

   BERR_Code retCode;
   SATFE_ConfigParam *pParam, *pParamIfStep;
   int32_t offset;
   uint32_t freq, n, i, m, acqs = 0, search_range, step_freq, num_freqs;
   hist_t *hist = NULL;
   bool bFound;
   uint8_t buf[4], idx;
   char key;

   BDBG_ASSERT(pChip);

   if (argc != 3)
   {
      SATFE_PrintDescription1("dft_test", "dft_test [freq] [n]", "Test DFT frequency estimate algorithm.", "freq = specifies RF tuning frequency", false);
      SATFE_PrintDescription2("n = number of acquisitions to run", true);
      return true;
   }

   /* get tuning frequency -> freq */
   if (SATFE_GetFreqFromString(pChip, argv[1], &freq) == false)
      return false;

   /* get number of acquisitions -> n */
   n = (uint32_t)atoi(argv[2]);


   /* get search_range -> search_range */
   retCode = BAST_GetSearchRange(pChip->hAst, &search_range);
   if (retCode)
   {
      printf("BAST_GetSearchRange() error 0x%X\n", retCode);
      return false;
   }

   step_freq = (uint32_t)((double)pChip->acqSettings[pChip->currAcqSettings].symbolRate * 0.05);
   if (step_freq > 1000000)
      step_freq = 1000000;
   m = (uint32_t)(((double)search_range * 2.0) / (double)step_freq) + 1;
   hist = (hist_t*)BKNI_Malloc(m * sizeof(hist_t));

   for (i = 0; i < m; i++)
   {
      hist[i].offset = -1;
      hist[i].count = 0;
   }
   num_freqs = 0;

   /* get freq_estimate_status config param */
   if (SATFE_LookupConfigParam(pChip, "freq_estimate_status", &pParam) == false)
   {
      printf("Unable to find freq_estimate_status configuration parameter\n");
      return false;
   }
   BDBG_ASSERT(pParam->len == 1);

   /* get if_step_save config param */
   if (SATFE_LookupConfigParam(pChip, "if_step_save", &pParamIfStep) == false)
   {
      printf("Unable to find if_step_save configuration parameter\n");
      return false;
   }
   BDBG_ASSERT(pParamIfStep->len == 4);

   key = 0;
   for (i = 0; (key <= 0) && (i < n) && (acqs < n); i++)
   {
      printf(".");
      fflush(stdout);
      /* clear freq_estimate_status */
      idx = 0;
      SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pParam->id, &idx, 1));
      if (retCode != BERR_SUCCESS)
         goto done;

      /* start the acquisition */
      SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, freq, &pChip->acqSettings[pChip->currAcqSettings]));
      if (retCode != BERR_SUCCESS)
      {
         printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
         goto done;
      }

      /* wait for acquisition to finish */
      SATFE_Platform_StartTimer();
      while ((SATFE_Platform_GetTimerCount() < 4000) && (key <= 0))
      {
         SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pParam->id, &idx, 1));
         if (retCode != BERR_SUCCESS)
            goto done;

         /* printf("status=0x%02X\n", idx); */
         if (idx & 0x80)
            break;
         key = SATFE_Platform_GetChar(false);
      }
      SATFE_Platform_KillTimer();
      if (key > 0)
         continue;

      if ((idx & 0x80) == 0)
      {
         printf("DFT timeout!\n");
         goto done;
      }

      /* read if_step_save */
      SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pParamIfStep->id, buf, 4));
      if (retCode != BERR_SUCCESS)
         goto done;
      offset = (int32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
      /* printf("offset=%d\n", offset); */

      bFound = false;
      for (i = 0; i < num_freqs; i++)
      {
         if (hist[i].offset == offset)
         {
            bFound = true;
            break;
         }
      }

      if (!bFound)
      {
         i = num_freqs;
         num_freqs++;
         hist[i].offset = offset;
      }
      hist[i].count++;
      acqs++;
   }

   printf("\nnumber of acquisitions = %d\n", acqs);
   for (i = 0; (acqs > 0) && (i < num_freqs); i++)
   {
      printf("%8d Hz: %d (%.1f%%)\n", hist[i].offset, hist[i].count, (double)hist[i].count*100.0/(double)acqs);
   }

   done:
   BKNI_Free((void*)hist);
   return true;
}


/******************************************************************************
 SATFE_Command_monitor_lock_events()
******************************************************************************/
bool SATFE_Command_monitor_lock_events(struct SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BKNI_EventHandle *hLockEvent = NULL;
   int i;
   bool bLocked;

   BDBG_ASSERT(pChip);

   if (pChip->commonCmds.send)
      return pChip->commonCmds.monitor_lock_events(pChip, argc, argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("monitor_lock_events", "monitor_lock_events", "Monitor lock events in real time.", "none", true);
      return true;
   }

   hLockEvent = BKNI_Malloc(pChip->chip.nDemods * sizeof(BKNI_EventHandle));
   for (i = 0; i < pChip->chip.nDemods; i++)
      BAST_GetLockStateChangeEventHandle(pChip->hAstChannel[i], &hLockEvent[i]);

   while ((retCode == BERR_SUCCESS) && (SATFE_Platform_GetChar(false) <= 0) && (retCode == BERR_SUCCESS))
   {
      SATFE_OnIdle();
      for (i = 0; i < pChip->chip.nDemods; i++)
      {
         if (BKNI_WaitForEvent(hLockEvent[i], 10) == BERR_SUCCESS)
         {
            SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[i], &bLocked));
            if (retCode == BERR_SUCCESS)
               printf("EVENT: Channel %d %s\n", i, bLocked ? "locked" : "not locked");
            else
               printf("BAST_GetLockStatus() error 0x%X\n", retCode);
         }
      }
   }

   BKNI_Free((void*)hLockEvent);
   return true;
}


/******************************************************************************
 SATFE_Command_plc_trk()
******************************************************************************/
bool SATFE_Command_plc_trk(SATFE_Chip *pChip, int argc, char **argv)
{
#define PLC_LOCK_TIMEOUT 20000
   BAST_ChannelStatus status;
   double init_damp, damp_low, damp_high, damp_step, damp, best_damp, x;
   BERR_Code retCode = BERR_SUCCESS;
   int retry;
   uint32_t freq, bw, bw_low, bw_high, bw_step, min_errors, best_bw, count, ber_window, est_time, num_errors;
   bool bLocked, bSuccess;
   SATFE_ConfigParam *pConfig_ldpc_ctl, *pConfig_plc_alt_trk_bw, *pConfig_plc_alt_trk_damp;
   uint8_t ldpc_ctl, sb, buf[4];
   char key;

   if (argc != 10)
   {
      SATFE_PrintDescription1("plc_trk",
                              "plc_trk [freq] [bw_low] [bw_high] [bw_step] [init_damp] [damp_low] [damp_hi] [damp_step] [ber_time]",
                              "Find optimal tracking PLC value.", "freq = tuner frequency", false);
      SATFE_PrintDescription2("bw_low = minimum PLC bandwidth in Hz", false);
      SATFE_PrintDescription2("bw_high = maximum PLC bandwidth in Hz", false);
      SATFE_PrintDescription2("bw_step = PLC bandwidth step size in Hz", false);
      SATFE_PrintDescription2("init_damp = initial damping factor", false);
      SATFE_PrintDescription2("damp_low = minimum damping factor", false);
      SATFE_PrintDescription2("damp_hi = maximum damping factor", false);
      SATFE_PrintDescription2("damp_step = damping factor step size", false);
      SATFE_PrintDescription2("ber_time = BER time in milliseconds", true);
      return true;
   }

   /* get input parameters */
   if ((bSuccess = SATFE_GetFreqFromString(pChip, argv[1], &freq)) == false)
      goto done1;

   bw_low = (uint32_t)atoi(argv[2]);
   bw_high = (uint32_t)atoi(argv[3]);
   bw_step = (uint32_t)atoi(argv[4]);
   init_damp = atof(argv[5]);
   damp_low = atof(argv[6]);
   damp_high = atof(argv[7]);
   damp_step = atof(argv[8]);
   ber_window = atoi(argv[9]);

   if ((ber_window < 5000) || (damp_high < damp_low) || (damp_low < 1.0) || (damp_high < 1.0) || (damp_step < 0.0625) || (bw_high < bw_low) || (bw_step < 100))
   {
      printf("bad parameter specified\n");
      goto done1;
   }

   est_time = (((bw_high - bw_low) / bw_step) + 1) * ber_window;
   x = ((damp_high - damp_low) / damp_step) + 1;
   est_time += ((int)x * ber_window);
   printf("estimated test time = %d secs (%.2f minutes)\n", est_time / 1000, (double)est_time / 60000.0);

   if (SATFE_LookupConfigParam(pChip, "ldpc_ctl", &pConfig_ldpc_ctl) == false)
   {
      printf("unable to find ldpc_ctl configuration parameter\n");
      bSuccess = false;
      goto done1;
   }
   if (SATFE_LookupConfigParam(pChip, "plc_alt_trk_bw", &pConfig_plc_alt_trk_bw) == false)
   {
      printf("unable to find plc_alt_trk_bw configuration parameter\n");
      bSuccess = false;
      goto done1;
   }
   if (SATFE_LookupConfigParam(pChip, "plc_alt_trk_damp", &pConfig_plc_alt_trk_damp) == false)
   {
      printf("unable to find plc_alt_trk_damp configuration parameter\n");
      bSuccess = false;
      goto done1;
   }

   /* set ldpc_ctl to use alt plc */
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pConfig_ldpc_ctl->id, &ldpc_ctl, pConfig_ldpc_ctl->len)));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig(LDPC_CTL) error 0x%X\n", retCode);
      bSuccess = false;
      goto done1;
   }
   sb = ldpc_ctl | 0x08;
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_ldpc_ctl->id, &sb, pConfig_ldpc_ctl->len)));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(LDPC_CTL) error 0x%X\n", retCode);
      bSuccess = false;
      goto done1;
   }

   /* set initial damping factor */
   sb = (uint8_t)((init_damp * 8.0) + 0.5);
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_trk_damp->id, &sb, pConfig_plc_alt_trk_damp->len)));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(ALT_TRK_PLC_DAMP) error 0x%X\n", retCode);
      bSuccess = false;
      goto done;
   }

   printf("press 'n' to skip current acquisition, press any other key to abort test\n\n");

   best_bw = bw_low;
   best_damp = init_damp;
   min_errors = 0xFFFFFFFF;
   key = 0;
   for (bw = bw_low; ((bw <= bw_high) && (retCode == BERR_SUCCESS) && (key <= 0)); bw += bw_step)
   {
      /* set plc bw */
      buf[0] = (uint8_t)((bw >> 24) & 0xFF);
      buf[1] = (uint8_t)((bw >> 16) & 0xFF);
      buf[2] = (uint8_t)((bw >> 8) & 0xFF);
      buf[3] = (uint8_t)(bw & 0xFF);
      SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_trk_bw->id, buf, pConfig_plc_alt_trk_bw->len)));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_WriteConfig(ALT_TRK_PLC_BW) error 0x%X\n", retCode);
         goto done;
      }

      /* acquire with new plc */
      SATFE_DO_RETRY(SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, freq, &pChip->acqSettings[pChip->currAcqSettings])));
      if (retCode != BERR_SUCCESS)
      {
         printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
         goto done;
      }

      /* wait for lock */
      bLocked = false;
      SATFE_Platform_StartTimer();
      while ((SATFE_Platform_GetTimerCount() < PLC_LOCK_TIMEOUT) && !bLocked && (key <= 0))
      {
         if ((key = SATFE_Platform_GetChar(false)) <= 0)
         {
            SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked)));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetLockStatus() error 0x%X\n", retCode);
               goto done;
            }
         }
      }
      SATFE_Platform_KillTimer();

      if (key == 'n')
      {
         key = 0;
         continue;
      }

      if (!bLocked)
         printf("BW=%7d,damp=%.3f:not_locked\n", bw, init_damp);
      else if (key <= 0)
      {
         /* reset the bert */
         SATFE_MUTEX(retCode = BAST_ResetStatus(pChip->hAstChannel[pChip->currChannel]));

         /* wait some time to collect errors */
         SATFE_Platform_StartTimer();
         while (((count = SATFE_Platform_GetTimerCount()) < ber_window) && bLocked && (key <= 0))
         {
            SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked)));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetLockStatus() error 0x%X\n", retCode);
               goto done;
            }

            key = SATFE_Platform_GetChar(false);
            if (key == 's')
            {
               SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status)));
               if (retCode != BERR_SUCCESS)
               {
                  printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
                  goto done;
               }
               printf(" corr+bad=%d, berr_errs=%d\n",
                  status.modeStatus.ldpc.corrBlocks + status.modeStatus.ldpc.badBlocks, status.berErrors);
               key = 0;
            }
            printf("\r%d %%", (count * 100) / ber_window + 1);
         }
         SATFE_Platform_KillTimer();

         printf("BW=%7d, damp=%.3f: ", bw, init_damp);
         if ((key <= 0) && bLocked)
         {
            SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status)));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
               goto done;
            }

            num_errors = status.modeStatus.ldpc.corrBlocks + status.modeStatus.ldpc.badBlocks;
            printf("corr+bad=%9d, ber_errs=%9d", num_errors, status.berErrors);
            if ((status.reacqCount == 0) && (num_errors < min_errors))
            {
               min_errors = num_errors;
               best_bw = bw;
            }
         }
         else if (!bLocked)
            printf("NOT LOCKED!");
         else if (key == 'n')
         {
            printf("ABORTED");
            key = 0;
         }
         printf("\n");
      }
   }

   if ((key > 0) || retCode)
      goto done;

   /* set to best_bw */
   printf("\n*** select BW=%d ***\n", best_bw);
   buf[0] = (uint8_t)((best_bw >> 24) & 0xFF);
   buf[1] = (uint8_t)((best_bw >> 16) & 0xFF);
   buf[2] = (uint8_t)((best_bw >> 8) & 0xFF);
   buf[3] = (uint8_t)(best_bw & 0xFF);
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_trk_bw->id, buf, pConfig_plc_alt_trk_bw->len)));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(LDPC_ALT_TRK_PLC_BW) error 0x%X\n", retCode);
      goto done;
   }

   min_errors = 0xFFFFFFFF;
   for (damp = damp_low; ((damp <= damp_high) && (retCode == BERR_SUCCESS) && (key <= 0)); damp += damp_step)
   {
      /* set new damping factor */
      sb = (uint8_t)((damp * 8.0) + 0.5);
      SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_trk_damp->id, &sb, pConfig_plc_alt_trk_damp->len)));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_WriteConfig(LDPC_ALT_TRK_PLC_DAMP) error 0x%X\n", retCode);
         goto done;
      }

      /* acquire with new damping factor */
      SATFE_DO_RETRY(SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, freq, &pChip->acqSettings[pChip->currAcqSettings])));
      if (retCode != BERR_SUCCESS)
      {
         printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
         goto done;
      }

      /* wait for lock */
      bLocked = false;
      SATFE_Platform_StartTimer();
      while ((SATFE_Platform_GetTimerCount() < PLC_LOCK_TIMEOUT) && !bLocked && (key <= 0))
      {
         if ((key = SATFE_Platform_GetChar(false)) <= 0)
         {
            SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked)));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetLockStatus() error 0x%X\n", retCode);
               goto done;
            }
         }
      }
      SATFE_Platform_KillTimer();

      if (key == 'n')
      {
         key = 0;
         continue;
      }

      if (!bLocked)
         printf("BW=%7d,damp=%.3f:not_locked\n", best_bw, damp);
      else if (key <= 0)
      {
         /* reset the bert */
         SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_ResetStatus(pChip->hAstChannel[pChip->currChannel])));

         /* wait some time to collect errors */
         SATFE_Platform_StartTimer();
         while (((count = SATFE_Platform_GetTimerCount()) < ber_window) && bLocked && (key <= 0))
         {
            SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked)));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetLockStatus() error 0x%X\n", retCode);
               goto done;
            }

            key = SATFE_Platform_GetChar(false);
            printf("\r%d %%", (count * 100) / ber_window + 1);
         }
         SATFE_Platform_KillTimer();

         printf("BW=%7d, damp=%.3f: ", best_bw, damp);
         if ((key <= 0) && bLocked)
         {
            SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status)));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetChannelStatus() error 0x%X\n", retCode);
               goto done;
            }

            num_errors = status.modeStatus.ldpc.corrBlocks + status.modeStatus.ldpc.badBlocks;
            printf("corr+bad=%9d, ber_errs=%9d", num_errors, status.berErrors);
            if ((status.reacqCount == 0) && (num_errors < min_errors))
            {
               min_errors = num_errors;
               best_damp = damp;
            }
         }
         else if (!bLocked)
            printf("NOT LOCKED!");
         else if (key == 'n')
         {
            printf("ABORTED");
            key = 0;
         }
         printf("\n");
      }
   }

   if ((key <= 0) && (retCode == BERR_SUCCESS))
      printf("\n*** select BW=%d, damp=%.3f ***\n", best_bw, best_damp);

   done:
   SATFE_Platform_KillTimer();

   /* restore ldpc_ctl */
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_ldpc_ctl->id, &ldpc_ctl, pConfig_ldpc_ctl->len)));
   if (retCode != BERR_SUCCESS)
      printf("BAST_WriteConfig(LDPC_CTL) error 0x%X\n", retCode);

   done1:
   return bSuccess;
}


/******************************************************************************
 SATFE_Command_plc_acq()
******************************************************************************/
bool SATFE_Command_plc_acq(SATFE_Chip *pChip, int argc, char **argv)
{
#define PLC_LOCK_TIMEOUT 20000
   BAST_ChannelStatus status;
   double init_damp, damp_low, damp_high, damp_step, damp, best_damp;
   BERR_Code retCode = BERR_SUCCESS;
   int retry;
   uint32_t freq, bw, bw_low, bw_high, bw_step, min_acq_time, min_reacqs, best_bw, num_acq, acq_time, i, total_acq_time, total_reacqs, lockStableTime;
   bool bLocked, bAborted, bSuccess;
   SATFE_ConfigParam *pConfig_ldpc_ctl, *pConfig_plc_alt_acq_bw, *pConfig_plc_alt_acq_damp;
   uint8_t ldpc_ctl, sb, buf[4];
   char key, str[40];

   if (argc != 10)
   {
      SATFE_PrintDescription1("plc_acq",
                              "plc_trk [freq] [bw_low] [bw_high] [bw_step] [init_damp] [damp_low] [damp_hi] [damp_step] [n]",
                              "Find optimal acquisition PLC value.", "freq = tuner frequency", false);
      SATFE_PrintDescription2("bw_low = minimum PLC bandwidth in Hz", false);
      SATFE_PrintDescription2("bw_high = maximum PLC bandwidth in Hz", false);
      SATFE_PrintDescription2("bw_step = PLC bandwidth step size in Hz", false);
      SATFE_PrintDescription2("init_damp = initial damping factor", false);
      SATFE_PrintDescription2("damp_low = minimum damping factor", false);
      SATFE_PrintDescription2("damp_hi = maximum damping factor", false);
      SATFE_PrintDescription2("damp_step = damping factor step size", false);
      SATFE_PrintDescription2("n = number of acquisitions per data point", true);
      return true;
   }

   /* get input parameters */
   if ((bSuccess = SATFE_GetFreqFromString(pChip, argv[1], &freq)) == false)
      goto done1;
   bw_low = (uint32_t)atoi(argv[2]);
   bw_high = (uint32_t)atoi(argv[3]);
   bw_step = (uint32_t)atoi(argv[4]);
   init_damp = atof(argv[5]);
   damp_low = atof(argv[6]);
   damp_high = atof(argv[7]);
   damp_step = atof(argv[8]);
   num_acq = atoi(argv[9]);

   if ((num_acq < 1) || (damp_high < damp_low) || (damp_low < 1.0) || (damp_high < 1.0) || (damp_step < 0.0625) || (bw_high < bw_low) || (bw_step < 100))
   {
      printf("bad parameter specified\n");
      goto done1;
   }

   if (SATFE_LookupConfigParam(pChip, "ldpc_ctl", &pConfig_ldpc_ctl) == false)
   {
      printf("unable to find ldpc_ctl configuration parameter\n");
      bSuccess = false;
      goto done1;
   }
   if (SATFE_LookupConfigParam(pChip, "plc_alt_acq_bw", &pConfig_plc_alt_acq_bw) == false)
   {
      printf("unable to find plc_alt_acq_bw configuration parameter\n");
      bSuccess = false;
      goto done1;
   }
   if (SATFE_LookupConfigParam(pChip, "plc_alt_acq_damp", &pConfig_plc_alt_acq_damp) == false)
   {
      printf("unable to find plc_alt_acq_damp configuration parameter\n");
      bSuccess = false;
      goto done1;
   }

   /* set ldpc_ctl to use alt plc */
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pConfig_ldpc_ctl->id, &ldpc_ctl, pConfig_ldpc_ctl->len)));
   if (retCode != BERR_SUCCESS)
   {
      bSuccess = false;
      printf("BAST_ReadConfig(LDPC_CTL) error 0x%X\n", retCode);
      goto done1;
   }
   sb = ldpc_ctl | 0x04;
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_ldpc_ctl->id, &sb, pConfig_ldpc_ctl->len)));
   if (retCode != BERR_SUCCESS)
   {
      bSuccess = false;
      printf("BAST_WriteConfig(LDPC_CTL) error 0x%X\n", retCode);
      goto done1;
   }

   /* set initial damping factor */
   sb = (uint8_t)((init_damp * 8.0) + 0.5);
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_acq_damp->id, &sb, pConfig_plc_alt_acq_damp->len)));
   if (retCode != BERR_SUCCESS)
   {
      bSuccess = false;
      printf("BAST_WriteConfig(ALT_ACQ_PLC_DAMP) error 0x%X\n", retCode);
      goto done;
   }

   printf("press 'n' to skip current acquisition, press any other key to abort test\n\n");

   best_bw = bw_low;
   best_damp = init_damp;
   min_acq_time = 0xFFFFFFFF;
   min_reacqs = 0xFFFFFFFF;
   key = 0;

   /* set lockStableTime */
   if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 4000000UL)
      lockStableTime = 1000;
   else if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 2000000UL)
      lockStableTime = 2000;
   else if (pChip->acqSettings[pChip->currAcqSettings].symbolRate > 1000000UL)
      lockStableTime = 3000;
   else
      lockStableTime = 5000;

   for (bw = bw_low; ((bw <= bw_high) && (retCode == BERR_SUCCESS) && (key <= 0)); bw += bw_step)
   {
      /* set plc bw */
      buf[0] = (uint8_t)((bw >> 24) & 0xFF);
      buf[1] = (uint8_t)((bw >> 16) & 0xFF);
      buf[2] = (uint8_t)((bw >> 8) & 0xFF);
      buf[3] = (uint8_t)(bw & 0xFF);
      SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_acq_bw->id, buf, pConfig_plc_alt_acq_bw->len)));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_WriteConfig(ALT_ACQ_PLC_BW) error 0x%X\n", retCode);
         goto done;
      }

      /* acquire with new acq plc */
      bLocked = true;
      total_acq_time = 0;
      total_reacqs = 0;
      for (i = 0; bLocked && (i < num_acq); i++)
      {
         printf("\r%.2f %%", ((double)(i+1) * 100.0) / (double)num_acq);

         SATFE_TimedAcquire(pChip, freq, &pChip->acqSettings[pChip->currAcqSettings], 10000, lockStableTime, &bAborted, &bLocked, &status, &acq_time);
         if (bAborted)
            break;
         if (bLocked)
         {
            total_reacqs += status.reacqCount;
            total_acq_time += acq_time;
         }
      }

      if (bAborted)
         strcpy(str, "aborted");
      else if (bLocked)
         strcpy(str, "locked");
      else
         strcpy(str, "not_locked");

      printf("BW=%8d,damp=%.3f:%s", bw, init_damp, str);

      if (!bLocked || bAborted)
      {
         printf("\n");
         continue;
      }
      printf(",%10.3f msecs,%4d reacqs\n", (double)total_acq_time / (double)num_acq, total_reacqs);
      if (total_acq_time < min_acq_time)
      {
         min_acq_time = total_acq_time;
         best_bw = bw;
      }
      if (total_reacqs < min_reacqs)
         min_reacqs = total_reacqs;
   }

   if ((key > 0) || retCode)
      goto done;

   /* set to best_bw */
   printf("\n*** select BW=%d ***\n", best_bw);
   buf[0] = (uint8_t)((best_bw >> 24) & 0xFF);
   buf[1] = (uint8_t)((best_bw >> 16) & 0xFF);
   buf[2] = (uint8_t)((best_bw >> 8) & 0xFF);
   buf[3] = (uint8_t)(best_bw & 0xFF);
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_acq_bw->id, buf, pConfig_plc_alt_acq_bw->len)));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig(ALT_ACQ_PLC_BW) error 0x%X\n", retCode);
      goto done;
   }

   min_acq_time = 0xFFFFFFFF;
   min_reacqs = 0xFFFFFFFF;
   for (damp = damp_low; ((damp <= damp_high) && (retCode == BERR_SUCCESS) && (key <= 0)); damp += damp_step)
   {
      /* set new damping factor */
      sb = (uint8_t)((damp * 8.0) + 0.5);
      SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_plc_alt_acq_damp->id, &sb, pConfig_plc_alt_acq_damp->len)));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_WriteConfig(ALT_ACQ_PLC_DAMP) error 0x%X\n", retCode);
         goto done;
      }

      /* acquire with new acq plc */
      bLocked = true;
      total_acq_time = 0;
      total_reacqs = 0;
      for (i = 0; bLocked && (i < num_acq); i++)
      {
         printf("\r%.2f %%", ((double)(i+1) * 100.0) / (double)num_acq);

         SATFE_TimedAcquire(pChip, freq, &pChip->acqSettings[pChip->currAcqSettings], 10000, lockStableTime, &bAborted, &bLocked, &status, &acq_time);
         if (bAborted)
            break;
         if (bLocked)
         {
            total_reacqs += status.reacqCount;
            total_acq_time += acq_time;
         }
      }

      if (bAborted)
         strcpy(str, "aborted");
      else if (bLocked)
         strcpy(str, "locked");
      else
         strcpy(str, "not_locked");

      printf("BW=%8d,damp=%.3f:%s", best_bw, damp, str);

      if (!bLocked || bAborted)
      {
         printf("\n");
         continue;
      }
      printf(",%10.3f msecs,%4d reacqs\n", (double)total_acq_time / (double)num_acq, total_reacqs);
      if (total_acq_time < min_acq_time)
      {
         min_acq_time = total_acq_time;
         best_damp = damp;
      }
      if (total_reacqs < min_reacqs)
         min_reacqs = total_reacqs;
   }

   if ((key <= 0) && (retCode == BERR_SUCCESS))
      printf("\n*** select BW=%d, damp=%.3f ***\n", best_bw, best_damp);

   done:
   SATFE_Platform_KillTimer();

   /* restore ldpc_ctl */
   SATFE_DO_RETRY(SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], pConfig_ldpc_ctl->id, &ldpc_ctl, pConfig_ldpc_ctl->len)));

   done1:
   return bSuccess;
}


/******************************************************************************
 SATFE_Command_tuner_filter()
******************************************************************************/
bool SATFE_Command_tuner_filter(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint32_t cutoffHz;

   if (argc != 2)
   {
      SATFE_PrintDescription1("tuner_filter",
                              "tuner_filter <cutoff_freq>",
                              "Set tuner filter.", "cutoff_freq = cutoff frequency.  If cutoff_freq is 0, then tuner filter setting is automatically set by firmware.", true);
      return true;
   }

   cutoffHz = (uint32_t)strtoul(argv[1], NULL, 10);
   if (cutoffHz != 0)
   {
      if (cutoffHz < 64)
         cutoffHz *= 1000000;
      if (cutoffHz > 63000000)
         cutoffHz = 63000000;
      else if (cutoffHz < 1000000)
         cutoffHz = 1000000;
   }
   SATFE_MUTEX(retCode = BAST_SetTunerFilter(pChip->hAstChannel[pChip->currChannel], cutoffHz));
   SATFE_RETURN_ERROR("BAST_SetTunerFilter()", retCode);
}


/******************************************************************************
 SATFE_Command_search_range()
******************************************************************************/
bool SATFE_Command_search_range(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint32_t search_range;

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("search_range",
                              "search_range <rangeHz>",
                              "Get/Set search range.", "rangeHz = search_range in Hz", true);
      return true;
   }

   if (argc == 1)
   {
      SATFE_MUTEX(retCode = BAST_GetSearchRange(pChip->hAst, &search_range));
      SATFE_CHECK_RETCODE("BAST_GetSearchRange()", retCode);
      printf("search range = %u Hz\n", search_range);
   }
   else
   {
      search_range = (uint32_t)atoi(argv[1]);
      SATFE_MUTEX(retCode = BAST_SetSearchRange(pChip->hAst, search_range));
      SATFE_CHECK_RETCODE("BAST_SetSearchRange()", retCode);
   }
   return true;
}


/******************************************************************************
 SATFE_Command_network_spec()
******************************************************************************/
bool SATFE_Command_network_spec(SATFE_Chip *pChip, int argc, char **argv)
{
   static char* SATFE_NETWORK_SPEC_STR[] =
   {
      "default", "echo", "euro"
   };

   BERR_Code retCode;
   BAST_NetworkSpec networkSpec;
   uint32_t i;

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("network_spec",
                              "network_spec <spec>",
                              "Get/Set network spec.",
                              "spec = {0: default, 1: echostar, 2: euro}", true);
      return true;
   }

   if (argc == 1)
   {
      SATFE_MUTEX(retCode = BAST_GetNetworkSpec(pChip->hAst, &networkSpec));
      SATFE_CHECK_RETCODE("BAST_GetNetworkSpec()", retCode);
      printf("network_spec = %u (%s)\n", networkSpec, SATFE_NETWORK_SPEC_STR[networkSpec]);
   }
   else
   {
      if (!strncmp(argv[1], "0x", 2))
         i = strtoul(argv[1], NULL, 16);
      else
         i = strtoul(argv[1], NULL, 10);
      if (i > (uint32_t)BAST_NetworkSpec_eEuro)
      {
         printf("invalid network_spec (must be 0, 1, or 2)\n");
         return false;
      }
      SATFE_MUTEX(retCode = BAST_SetNetworkSpec(pChip->hAst, (BAST_NetworkSpec)i));
      SATFE_CHECK_RETCODE("BAST_SetNetworkSpec()", retCode);
   }
   return true;
}


/******************************************************************************
 SATFE_Command_test_status_indicators()
******************************************************************************/
bool SATFE_Command_test_status_indicators(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_ChannelStatus channel_status;
   BKNI_EventHandle statusEvent;
   SATFE_ConfigParam *pConfig_rain_fade_threshold, *pConfig_status_indicator, *pConfig_freq_drift_threshold;
   uint32_t val;
   uint8_t buf[4];
   char key;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("test_status_indicators",
                              "test_status_indicators",
                              "This utility is used to verify the status indicators.", "none", true);
      return true;
   }

   if (SATFE_LookupConfigParam(pChip, "status_indicator", &pConfig_status_indicator) == false)
   {
      printf("unable to find status_indicator configuration parameter\n");
      goto done1;
   }

   if (SATFE_LookupConfigParam(pChip, "rain_fade_threshold", &pConfig_rain_fade_threshold) == false)
   {
      printf("unable to find rain_fade_threshold configuration parameter\n");
      goto done1;
   }

   if (SATFE_LookupConfigParam(pChip, "freq_drift_threshold", &pConfig_freq_drift_threshold) == false)
   {
      printf("unable to find freq_drift_threshold configuration parameter\n");
      goto done1;
   }

   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pConfig_rain_fade_threshold->id, (uint8_t*)buf, pConfig_rain_fade_threshold->len));
   SATFE_GOTO_DONE_ON_ERROR("BAST_ReadConfig(rain_fade_threshold)", retCode);
   val = buf[0];
   printf("rain fade threshold  = %.2f dB\n", (float)val / 8.0);

   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pConfig_freq_drift_threshold->id, (uint8_t*)buf, pConfig_freq_drift_threshold->len));
   SATFE_GOTO_DONE_ON_ERROR("BAST_ReadConfig(freq_drift_threshold)", retCode);
   val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
   printf("freq drift threshold  = %d Hz\n", val);

   retCode = BAST_GetStatusEventHandle(pChip->hAstChannel[pChip->currChannel], &statusEvent);
   SATFE_GOTO_DONE_ON_ERROR("BAST_GetStatusEventHandle()", retCode);
   SATFE_MUTEX(retCode = BAST_EnableStatusInterrupts(pChip->hAstChannel[pChip->currChannel], true));
   SATFE_GOTO_DONE_ON_ERROR("BAST_EnableStatusInterrupts(true)", retCode);

   while (1)
   {
      key = SATFE_WaitForKeyPress(10);
      if (key > 0)
         break;

      SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &channel_status));
      SATFE_GOTO_DONE_ON_ERROR("BAST_GetChannelStatus()", retCode);
      printf("%s: SNR=%2.3f, carr_error=%d\t   %c",
             channel_status.bDemodLocked ? "LOCKED" : "NOT LOCKED", channel_status.snrEstimate / 256.0, channel_status.carrierError,
             channel_status.bDemodLocked ? '\r' : '\n');

      if (BKNI_WaitForEvent(statusEvent, 0) == BERR_SUCCESS)
      {
         SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], pConfig_status_indicator->id, (uint8_t*)buf, pConfig_status_indicator->len));
         SATFE_GOTO_DONE_ON_ERROR("BAST_ReadConfig(status_indicator)", retCode);

         if (buf[0] & 0x0F)
         {
            printf("\n");
            if (buf[0] & 0x01)
               printf("rain_fade ");
            if (buf[0] & 0x02)
               printf("freq_drift ");
            if (buf[0] & 0x04)
               printf("thresh2 ");
            if (buf[0] & 0x08)
               printf("thresh1");
            printf("\n");
         }
      }
   }

   done:
   printf("\n\n");
   BAST_EnableStatusInterrupts(pChip->hAstChannel[pChip->currChannel], false);

   done1:
   SATFE_RETURN_ERROR("SATFE_Command_test_status_indicators()", retCode);
}


/******************************************************************************
 SATFE_Command_debug()
******************************************************************************/
bool SATFE_Command_debug(SATFE_Chip *pChip, int argc, char **argv)
{
   static char *SATFE_ModuleLevelString[] = {
     "unknown",
     "trace",
     "msg",
     "wrn",
     "err"
   };

   BDBG_Level lvl;
   int nModules, i;
   char **moduleNames;
   bool bModule;

   if (pChip->chipFunctTable->GetDebugModuleNames == NULL)
   {
      printf("GetDebugModuleNames function not implemented for this chip!\n");
      return false;
   }

   pChip->chipFunctTable->GetDebugModuleNames(pChip, &nModules, &moduleNames);
   if ((argc != 2) && (argc != 3))
   {
      SATFE_PrintDescription1("debug",
                              "debug <module_name> [level]",
                              "Enables/Disables AST PI source code debug messages.",
                              "level = {on, off, trace, msg, wrn, err}", false);
      SATFE_PrintDescription2("off = disable debug messages", false);
      printf("   module_name = one of the following:\n");
      for (i = 0; i < nModules; i++)
      {
         BDBG_GetModuleLevel(moduleNames[i], &lvl);
         printf("      %s (current debug level is %s)\n", moduleNames[i], SATFE_ModuleLevelString[lvl]);
      }
      printf("------------------------------------------------------------------------------\n");
      return true;
   }

   if (argc == 3)
   {
      bModule = true;
      for (i = 0; i < nModules; i++)
      {
         if (!strcmp(moduleNames[i], argv[1]))
            break;
      }
      if (i >= nModules)
      {
         printf("ERROR: invalid debug module name\n");
         return false;
      }
   }
   else
   {
      bModule = false;
   }
   if (!strcmp(argv[argc-1], "on") || !strcmp(argv[argc-1], "msg"))
      lvl = BDBG_eMsg;
   else if (!strcmp(argv[argc-1], "off") || !strcmp(argv[argc-1], "err"))
      lvl = BDBG_eErr;
   else if (!strcmp(argv[argc-1], "trace"))
      lvl = BDBG_eTrace;
   else if (!strcmp(argv[argc-1], "wrn"))
      lvl = BDBG_eWrn;
   else
   {
      printf("invalid parameter (%s)\n", argv[argc-1]);
      return false;
   }

   if (bModule)
      BDBG_SetModuleLevel(argv[1], lvl);
   else
   {
      for (i = 0; i < nModules; i++)
      {
         BDBG_SetModuleLevel(moduleNames[i], lvl);
      }
   }
   return true;
}


static uint32_t nPsdCandidates = 0;


/******************************************************************************
 SATFE_Command_reboot()
******************************************************************************/
BERR_Code SATFE_Command_reboot(SATFE_Chip *pChip, int argc, char **argv)
{
   typedef void (*RESET_FUNC)();
   RESET_FUNC reset_func = (RESET_FUNC)0xBFC00000;

   if (argc != 1)
   {
      SATFE_PrintDescription1("reboot", "reboot", "Reboot the chip.", "none", true);
      return true;
   }

   reset_func();
   return BERR_SUCCESS; /* never returns */
}


/******************************************************************************
 SATFE_Command_dft_debug() - make sure BAST_DUMP_BINS is defined in bast_g3_priv_dft.c
******************************************************************************/
BERR_Code SATFE_Command_dft_debug(SATFE_Chip *pChip, int argc, char **argv)
{
   extern BERR_Code BAST_g3_P_DumpBins(BAST_ChannelHandle h, uint32_t tunerFreq);

   uint32_t start_freq, stop_freq, step_freq, start_baud, stop_baud, freq;
   float f;
   BERR_Code retCode;
   BAST_PeakScanStatus peakStatus;
   BKNI_EventHandle peakScanEvent;
   int i;
   uint8_t misc_ctl, runTimes;

   if (argc != 7)
   {
      SATFE_PrintDescription1("dft_debug [start_freq] [stop_freq] [freq_step] [start_baud] [stop_baud] [runTimes]", "dft_debug", "Dump out dft bin data.", "none", true);
      return true;
   }

   /* process command line arguments */
   if (SATFE_GetFreqFromString(pChip, argv[1], &start_freq) == false)
      return false;
   if (SATFE_GetFreqFromString(pChip, argv[2], &stop_freq) == false)
      return false;
   f = (float)atof(argv[3]);
   if (f < 50)
      step_freq = (uint32_t)(f * 1000000);
   else
      step_freq = (uint32_t)f;
   f = (float)atof(argv[4]);
   if (f < 50)
      start_baud = (uint32_t)(f * 1000000);
   else
      start_baud = (uint32_t)f;
   f = (float)atof(argv[5]);
   if (f < 50)
      stop_baud = (uint32_t)(f * 1000000);
   else
      stop_baud = (uint32_t)f;
   runTimes = (uint8_t)strtoul(argv[6], NULL, 10);
   if ((runTimes == 0) || (runTimes > 63))
   {
      printf("runTimes out of range!\n");
      return false;
   }

   if ((start_baud < 256000UL) || (start_baud > 45000000UL))
   {
      printf("start_baud is out of range\n");
      return false;
   }
   if ((stop_baud < 256000UL) || (stop_baud > 45000000UL))
   {
      printf("stop_baud is out of range\n");
      return false;
   }
   if (start_baud >= stop_baud)
   {
      printf("ERROR: start_baud >= stop_baud\n");
      return false;
   }
   if (start_baud >= stop_baud)
   {
      printf("ERROR: start_baud >= stop_baud\n");
      return false;
   }
   if (start_freq >= stop_freq)
   {
      printf("ERROR: start_freq >= stop_freq\n");
      return false;
   }

   /* obtain the peak scan done event handle */
   BAST_GetPeakScanEventHandle(pChip->hAstChannel[pChip->currChannel], &peakScanEvent);

   SATFE_MUTEX(retCode = BAST_SetPeakScanSymbolRateRange(pChip->hAstChannel[pChip->currChannel], start_baud, stop_baud));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_SetPeakScanSymbolRateRange() error 0x%X\n", retCode);
      return retCode;
   }

   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig() error 0x%X\n", retCode);
      return retCode;
   }
   misc_ctl |= (BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD | BAST_G3_CONFIG_MISC_CTL_DISABLE_SMART_TUNE);
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig() error 0x%X\n", retCode);
      return retCode;
   }

   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_DFT_MIN_N, &runTimes, BAST_G3_CONFIG_LEN_DFT_MIN_N));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig() error 0x%X\n", retCode);
      return retCode;
   }

   for (freq = start_freq; freq < stop_freq; freq += step_freq)
   {
      SATFE_MUTEX(retCode = BAST_g3_P_DumpBins(pChip->hAstChannel[pChip->currChannel], freq));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_PeakScan() error 0x%X\n", retCode);
         return false;
      }

      /* wait for the peak_scan event */
#if defined(DIAGS) || defined(WIN32)
      for (i = 0; i < 5000; i++)
      {
         if ((retCode = BKNI_WaitForEvent(peakScanEvent, 0)) == BERR_SUCCESS)
            break;
         BKNI_Sleep(1);
      }
#else
      retCode = BKNI_WaitForEvent(peakScanEvent, 5000);
#endif
      if (retCode != BERR_SUCCESS)
      {
         printf("peak scan event not received!\n");
         return retCode;
      }

      /* get the results of the symbol rate scan */
      SATFE_MUTEX(retCode = BAST_GetPeakScanStatus(pChip->hAstChannel[pChip->currChannel], &peakStatus));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetPeakScanStatus() error 0x%X\n", retCode);
         return retCode;
      }

      /* verify that the peak scan finished successfully */
      if (peakStatus.status != 0)
      {
         printf("ERROR: peak scan status = %d\n", peakStatus.status);
         return BERR_UNKNOWN;
      }
   }

   return true;
}


/******************************************************************************
 SATFE_ScanSymbolRate()
******************************************************************************/
BERR_Code SATFE_ScanSymbolRate(SATFE_Chip *pChip, uint32_t Fc, uint32_t range, uint32_t step_size, uint32_t min_symbol_rate, uint32_t max_symbol_rate, BAST_PeakScanStatus *pPeakStatus, uint32_t *pLastFreq, bool bVerbose)
{
#define CONSECUTIVE_SYM_RATE_THRESHOLD 2
   BERR_Code retCode;
   uint32_t tuner_freq, last_symbol_rate = 0, symbol_rate_count, max_peak, max_peak_freq, i;
   BAST_PeakScanStatus peakStatus;
   BKNI_EventHandle peakScanEvent;
   uint8_t misc_ctl;

   /* obtain the peak scan done event handle */
   BAST_GetPeakScanEventHandle(pChip->hAstChannel[pChip->currChannel], &peakScanEvent);

   /* switch off psd mode in peakscan */
   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig() error 0x%X\n", retCode);
      return retCode;
   }
   misc_ctl &= ~BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD;
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig() error 0x%X\n", retCode);
      return retCode;
   }

   pPeakStatus->status = 0;
   pPeakStatus->peakPower = 0;
   pPeakStatus->tunerFreq = 0;
   pPeakStatus->out = 0;

   if (bVerbose)
      printf("*** %d-%d Hz: minFb=%d, maxFb=%d, step=%d\n", Fc - range, Fc + range, min_symbol_rate, max_symbol_rate, step_size);
   SATFE_MUTEX(retCode = BAST_SetPeakScanSymbolRateRange(pChip->hAstChannel[pChip->currChannel], min_symbol_rate, max_symbol_rate));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_SetPeakScanSymbolRateRange() error 0x%X\n", retCode);
      return retCode;
   }

   symbol_rate_count = 0;
   max_peak = 0;
   max_peak_freq = 0;
   last_symbol_rate = 0;

   for (tuner_freq = (Fc - range); tuner_freq < (Fc + range); tuner_freq += step_size)
   {
      *pLastFreq = tuner_freq;

      /* initiate symbol rate scan */
      SATFE_MUTEX(retCode = BAST_PeakScan(pChip->hAstChannel[pChip->currChannel], tuner_freq));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_PeakScan() error 0x%X\n", retCode);
         goto done;
      }

      /* wait for the peak_scan event */
#if defined(DIAGS) || defined(WIN32)
      for (i = 0; i < 1000; i++)
      {
         if ((retCode = BKNI_WaitForEvent(peakScanEvent, 0)) == BERR_SUCCESS)
            break;
         BKNI_Sleep(1);
      }
#else
      retCode = BKNI_WaitForEvent(peakScanEvent, 1000);
#endif
      if (retCode != BERR_SUCCESS)
      {
         printf("peak scan event not received!\n");
         goto done;
      }

      /* get the results of the symbol rate scan */
      SATFE_MUTEX(retCode = BAST_GetPeakScanStatus(pChip->hAstChannel[pChip->currChannel], &peakStatus));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetPeakScanStatus() error 0x%X\n", retCode);
         goto done;
      }

      /* verify that the peak scan finished successfully */
      if (peakStatus.status != 0)
      {
         printf("ERROR: peak scan status = %d\n", peakStatus.status);
         retCode = BERR_UNKNOWN;
         goto done;
      }

      if (bVerbose) printf("%u Hz: Fb=%d, power=%u\n", peakStatus.tunerFreq, peakStatus.out, peakStatus.peakPower);

      if ((symbol_rate_count == 0) && ((peakStatus.out == 0) || (peakStatus.peakPower == 0)))
         goto not_found;

      if (abs(peakStatus.out - last_symbol_rate) < (peakStatus.out >> 9))
      {
         /* peakStatus.out is approx equal to last_symbol_rate */
         symbol_rate_count++;
         if (peakStatus.peakPower > max_peak)
         {
            max_peak = peakStatus.peakPower;
            max_peak_freq = peakStatus.tunerFreq;
         }
      }
      else
      {
         /* symbol rate changed */
         if (symbol_rate_count >= (CONSECUTIVE_SYM_RATE_THRESHOLD-1))
         {
            found_signal:
            pPeakStatus->status = 0;
            pPeakStatus->peakPower = max_peak;
            pPeakStatus->tunerFreq = max_peak_freq;
            pPeakStatus->out = last_symbol_rate;
            goto done;
         }

         not_found:
         last_symbol_rate = peakStatus.out;
         symbol_rate_count = 0;
         max_peak = 0;
         max_peak_freq = 0;
      }
   }

   if (symbol_rate_count >= (CONSECUTIVE_SYM_RATE_THRESHOLD-1))
      goto found_signal;

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 log2Approx() - Return log2Result, which is approximately log2(xint) * 16
******************************************************************************/
uint32_t log2Approx(int xint)
{
   int msb = 0;
   int log2Result;

   int OutputShift = 4;    // Log2 output has 4 LSB after the point.

   if (xint<=1)
      return 0;
   else
   {
      while (xint >> msb) msb++;
      msb--;

      if(msb-OutputShift > 0)
         log2Result = (xint>>(msb-OutputShift))+((msb-1)<<OutputShift);
      else
         log2Result = (xint<<(OutputShift-msb))+((msb-1)<<OutputShift);

      //printf("%2d %2d %4d %4d\n", xint, msb, log2Result, (int)(log2(1.0*xint)*16));
      return (log2Result);
   }
}


/******************************************************************************
 SATFE_ScanSymbolRatePsd()
******************************************************************************/
BERR_Code SATFE_ScanSymbolRatePsd(SATFE_Chip *pChip, uint32_t Fc, uint32_t range, uint32_t step_size, uint32_t min_symbol_rate, uint32_t max_symbol_rate, BAST_PeakScanStatus *pPeakStatus, uint32_t *pLastFreq, bool bVerbose)
{
   static const int32_t incrementThreshold = 10;
   static const int32_t inBandStepThreshold = 40;
   BERR_Code retCode;
   int32_t psdData_dB[3], psdRaw_dB[5], sortBuf[5];
   uint32_t tuner_freq, i, psdFreq[3], risingFreq = 0, psdIndex = 0, psdRawIndex = 0, psdDataSize = 0;
   uint32_t maxStep = 0, maxStep_D1 = 0, maxStep_D2 = 0, maxStep_D3 = 0, freqPointIndex = 0, step, val;
   uint32_t candidateBaudRate = 0, candidateCenterFreq = 0, minFb, maxFb, rangeFb;
   BAST_PeakScanStatus peakStatus;
   BKNI_EventHandle peakScanEvent;
   uint8_t misc_ctl, risingType = 0;

   /* obtain the peak scan done event handle */
   BAST_GetPeakScanEventHandle(pChip->hAstChannel[pChip->currChannel], &peakScanEvent);

   pPeakStatus->status = 0;
   pPeakStatus->peakPower = 0;
   pPeakStatus->tunerFreq = 0;
   pPeakStatus->out = 0;

   if (bVerbose)
      printf("*** %u-%u Hz: minFb=%d, maxFb=%d, step=%d, range=%u\n", Fc - range, Fc + range, min_symbol_rate, max_symbol_rate, step_size, range);

   /* prepare peakscan for PSD mode */
   SATFE_MUTEX(retCode = BAST_SetPeakScanSymbolRateRange(pChip->hAstChannel[pChip->currChannel], 0, 0));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_SetPeakScanSymbolRateRange() error 0x%X\n", retCode);
      return retCode;
   }
   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig() error 0x%X\n", retCode);
      return retCode;
   }
   misc_ctl |= BSAT_G3_CONFIG_MISC_CTL_PEAKSCAN_PSD;
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_MISC_CTL, &misc_ctl, BAST_G3_CONFIG_LEN_MISC_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig() error 0x%X\n", retCode);
      return retCode;
   }

   for (tuner_freq = (Fc - range); tuner_freq < (Fc + range); tuner_freq += step_size)
   {
      *pLastFreq = tuner_freq;
      SATFE_MUTEX(retCode = BAST_PeakScan(pChip->hAstChannel[pChip->currChannel], tuner_freq));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_PeakScan() error 0x%X\n", retCode);
         return false;
      }

      /* wait for the peak_scan event */
#if defined(DIAGS) || defined(WIN32)
      for (i = 0; i < 1000; i++)
      {
         if ((retCode = BKNI_WaitForEvent(peakScanEvent, 0)) == BERR_SUCCESS)
            break;
         BKNI_Sleep(1);
      }
#else
      retCode = BKNI_WaitForEvent(peakScanEvent, 1000);
#endif
      if (retCode != BERR_SUCCESS)
      {
         printf("peak scan event not received!\n");
         goto done;
      }

      /* get the PSD data for this frequency */
      SATFE_MUTEX(retCode = BAST_GetPeakScanStatus(pChip->hAstChannel[pChip->currChannel], &peakStatus));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetPeakScanStatus() error 0x%X\n", retCode);
         goto done;
      }

      /* verify that the peak scan finished successfully */
      if (peakStatus.status != 0)
      {
         printf("ERROR: peak scan status = %d\n", peakStatus.status);
         retCode = BERR_UNKNOWN;
         goto done;
      }

      psdDataSize++;
      psdFreq[psdIndex] = tuner_freq;
      val = log2Approx(peakStatus.peakPower);
      psdRaw_dB[psdRawIndex] = (12 * val) / 16; /* psdData_dB[i] = 8*1.5*log2Approx(peak_pow) */

      if (psdDataSize >= 3)
      {
         bool bSwapped = true;
         int j = 0, tmp;

         /* determine the median of the last 5 points */
         for (i = 0; i < 5; i++)
            sortBuf[i] = psdRaw_dB[i];
         while (bSwapped)
         {
            bSwapped = false;
            j++;
            for (i = 0; i < (5-j); i++)
            {
               if (sortBuf[i] > sortBuf[i+1])
               {
                  tmp = sortBuf[i];
                  sortBuf[i] = sortBuf[i+1];
                  sortBuf[i+1] = tmp;
                  bSwapped = true;
               }
            }
         }
         psdData_dB[psdIndex] = sortBuf[2];
      }
      else
         psdData_dB[psdIndex] = 0;

      if (bVerbose)
         printf("%u Hz: peak=%u, Raw_dB=%u, Med_dB=%u, risingType=%d\n", psdFreq[psdIndex], peakStatus.peakPower, psdRaw_dB[psdRawIndex], psdData_dB[psdIndex], risingType);

      if (psdDataSize >= 5)
      {
         if (risingType > 0)
         {
            /* already found a rising edge */
            freqPointIndex++;
            maxStep_D3 = maxStep_D2;
            maxStep_D2 = maxStep_D1;
            maxStep_D1 = maxStep;
            step = abs(psdData_dB[psdIndex] - psdData_dB[(psdIndex+2)%3]);
            if ((step > maxStep) && (freqPointIndex > 3))
               maxStep = step;
            if ((psdData_dB[(psdIndex+1)%3] - psdData_dB[psdIndex]) >= incrementThreshold)
            {
               /* found a falling edge */
               candidateBaudRate = psdFreq[psdIndex] - risingFreq + (2*step_size); /*500000;*/
               if (candidateBaudRate >= 25000000)
                  candidateBaudRate -= 2500000;

               if ((candidateBaudRate >= min_symbol_rate) && (candidateBaudRate <= max_symbol_rate) && (maxStep_D3 < inBandStepThreshold))
               {
                  found_candidate:
                  nPsdCandidates++; /* for information only */
                  candidateCenterFreq = (psdFreq[psdIndex] + risingFreq) / 2;
                  candidateCenterFreq -= (2*step_size);

                  if (bVerbose) printf(">>> candidate found: Fc=%u, Fb=%u\n", candidateCenterFreq, candidateBaudRate);

                  if (candidateBaudRate >= 25000000)
                     rangeFb = 2000000;
                  else if (candidateBaudRate >= 5000000)
                     rangeFb = 1000000;
                  else
                     rangeFb = 500000;

                  if (rangeFb > candidateBaudRate)
                     minFb = min_symbol_rate;
                  else
                  {
                     minFb = candidateBaudRate - rangeFb;
                     if (minFb < min_symbol_rate)
                        minFb = min_symbol_rate;
                  }
                  maxFb = candidateBaudRate + rangeFb;
                  if (maxFb > max_symbol_rate)
                     maxFb = max_symbol_rate;
                  retCode = SATFE_ScanSymbolRate(pChip, candidateCenterFreq, 1000000, step_size, minFb, maxFb, pPeakStatus, &val, bVerbose);
                  return retCode;
               }
               else
               {
                  /* found a channel, but it is not vaild, reset the rising edge type */
                  risingType = 0;
                  freqPointIndex = 0;
               }
            }
            else
            {
               /* already found a rising edge, no falling edge yet */
               if ((psdData_dB[psdIndex] - psdData_dB[(psdIndex+1)%3]) >= incrementThreshold)
               {
                  /* found rising edge by crossing the incrementThreshold */
                  if ((psdFreq[psdIndex] - risingFreq) <= 1000000)
                  {
                     /* found a new rising edge that is 1MHz away from the existing risingType=1 edge */
                     goto found_new_rising_edge;
                  }
                  else if ((psdFreq[psdIndex] - risingFreq) >= 2000000)
                  {
                     /* found a new rising edge 2MHz away without finding a falling edge;
                        for existing rising edge, assume it is also a falling edge */
                     candidateBaudRate = psdFreq[psdIndex] - step_size - risingFreq;
                     if ((candidateBaudRate >= min_symbol_rate) && (candidateBaudRate <= max_symbol_rate) && (maxStep_D3 < inBandStepThreshold))
                        goto found_candidate;

                     risingType = 1;
                     risingFreq = psdFreq[psdIndex];
                     freqPointIndex = 1;
                     maxStep = 0;
                  }
               }
            }
         }
         else if ((psdData_dB[psdIndex] - psdData_dB[(psdIndex+1)%3]) >= incrementThreshold)
         {
            found_new_rising_edge:
            /* found rising edge by crossing the incrementThreshold */
            risingType = 1;
            risingFreq = psdFreq[psdIndex];
            freqPointIndex = 1;
            maxStep = 0;
            maxStep_D1 = 0;
            maxStep_D2 = 0;
            maxStep_D3 = 0;
         }
      }

      psdIndex = (psdIndex + 1) % 3;
      psdRawIndex = (psdRawIndex + 1) % 5;
   }

   done:
   return retCode;
}


/******************************************************************************
 SATFE_Command_blind_scan()
******************************************************************************/
bool SATFE_Command_blind_scan(SATFE_Chip *pChip, int argc, char **argv)
{
#if 0
   const uint32_t peak_scan_range = 2000000;
   const uint32_t peak_scan_carr_freq_step_size = 250000;
   BAST_SignalDetectStatus signalDetectStatus;
   BAST_AcqSettings acq_params;
   BAST_ChannelStatus status;
   BERR_Code retCode;
   uint32_t min_symbol_rate, max_symbol_rate, min_freq = 0, max_freq = 0;
   bool bVerbose = true, bSingleShot = false, bDontAcquire = false;
   uint32_t curr_tuner_freq, last_freq, last_found_freq = 0, last_found_symbol_rate = 0, nFound = 0, search_range;
   BAST_PeakScanStatus peakStatus;
   int i;
   bool bLocked = false;
   float f;

   if (argc < 5)
   {
      SATFE_PrintDescription1("blind_scan",
                              "blind_scan [min_freq] [max_freq] [min_sym_rate] [max_sym_rate] <-q> <-a> <-l>",
                              "Scan and lock to a signal within a given frequency range.",
                              "min_freq = minimum RF frequency in the scan range", false);
      SATFE_PrintDescription2("max_freq = maximum RF frequency in the scan range.", false);
      SATFE_PrintDescription2("min_sym_rate = minimum symbol rate in sym/sec", false);
      SATFE_PrintDescription2("max_sym_rate = maximum symbol rate in sym/sec", false);
      SATFE_PrintDescription2("-q = suppress debug output", false);
      SATFE_PrintDescription2("-a = exit on first transponder found", false);
      SATFE_PrintDescription2("-l = do not acquire potential signal found by symbol rate scan", true);

      return true;
   }

   /* process command line arguments */
   if (SATFE_GetFreqFromString(pChip, argv[1], &min_freq) == false)
      return false;
   if (SATFE_GetFreqFromString(pChip, argv[2], &max_freq) == false)
      return false;
   f = (float)atof(argv[3]);
   if (f < 50)
      min_symbol_rate = (uint32_t)(f * 1000000);
   else
      min_symbol_rate = (uint32_t)f;
   if ((min_symbol_rate < 256000UL) || (min_symbol_rate > 45000000UL))
   {
      printf("min_symbol_rate is out of range\n");
      return false;
   }
   f = (float)atof(argv[4]);
   if (f < 50)
      max_symbol_rate = (uint32_t)(f * 1000000);
   else
      max_symbol_rate = (uint32_t)f;
   if ((max_symbol_rate < 256000UL) || (max_symbol_rate > 45000000UL) || (max_symbol_rate <= min_symbol_rate))
   {
      printf("max_symbol_rate is out of range\n");
      return false;
   }

   for (i = 5; i < argc; i++)
   {
      if (!strcmp(argv[i], "-q"))
         bVerbose = false;
      else if (!strcmp(argv[i], "-a"))
         bSingleShot = true;
      else if (!strcmp(argv[i], "-l"))
         bDontAcquire = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   if (bVerbose)
   {
      printf("sweep freq from %u to %u in steps of %u\n", min_freq, max_freq, peak_scan_range*2);
      printf("min symbol rate = %u, max symbol rate = %u\n", min_symbol_rate, max_symbol_rate);
   }

   /* search from min_freq to max_freq */
   SATFE_Platform_StartTimer();
   for (curr_tuner_freq = min_freq + peak_scan_range; curr_tuner_freq <= (max_freq + peak_scan_range); )
   {
      /* check if user wants to abort */
      SATFE_OnIdle();
      if (SATFE_Platform_GetChar(false) > 0)
      {
         printf("operation aborted by user!\n");
         break;
      }

      retCode = SATFE_ScanSymbolRate(pChip, curr_tuner_freq, peak_scan_range, peak_scan_carr_freq_step_size, min_symbol_rate, max_symbol_rate, &peakStatus, &last_freq, bVerbose);
      if (retCode != BERR_SUCCESS)
         return false;

      bLocked = false;
      if (peakStatus.tunerFreq > 0)
      {
         printf("potential signal found at %d Hz (%d sym/s)\n", peakStatus.tunerFreq, peakStatus.out);

#if 1
         /* check if timing loop can lock */
         acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT | BAST_ACQSETTINGS_SIGNAL_DETECT_MODE;
         acq_params.carrierFreqOffset = 0;
         acq_params.mode = BAST_Mode_eDvb_scan; /* not used */
         acq_params.symbolRate = peakStatus.out;
         SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, peakStatus.tunerFreq, &acq_params));
         if (retCode != BERR_SUCCESS)
         {
            printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
            goto done;
         }

         /* wait for signal detection done */
         for (i = 1000; i; i--)
         {
            SATFE_MUTEX(retCode = BAST_GetSignalDetectStatus(pChip->hAstChannel[pChip->currChannel], &signalDetectStatus));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_GetSignalDetectStatus() error 0x%X\n", retCode);
               goto done;
            }

            /* sanity check */
            if (signalDetectStatus.bEnabled == false)
            {
               printf("BAST_GetSignalDetectStatus() error: bEnabled should not be false!\n");
               goto done;
            }

            if (signalDetectStatus.bDone)
               break;

#if 0
            SATFE_OnIdle();
            if (SATFE_Platform_GetChar(false) > 0)
            {
               printf("operation aborted by user!\n");
               goto done;
            }
#endif
            BKNI_Sleep(1);
         }
#endif
         if (i > 0)
         {
            if (!signalDetectStatus.bTimingLoopLocked)
            {
               printf("Unable to lock timing loop\n");
               goto determine_next_freq;
            }
            else
               printf("Timing loop is locked\n");
         }
         else
            printf("ERROR: signal detect function timeout\n");

         if (!bDontAcquire)
         {
            /* limit search range for low symbol rates */
            if (peakStatus.out > 4000000)
               search_range = 5000000;
            else
               search_range = peakStatus.out / 2;
            SATFE_MUTEX(retCode = BAST_SetSearchRange(pChip->hAst, search_range));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_SetSearchRange() error 0x%X\n", retCode);
               goto done;
            }

            /* do a blind acquisition on this (freq,symbol_rate) */
            acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
            acq_params.carrierFreqOffset = 0;
            acq_params.mode = BAST_Mode_eBlindScan;
            acq_params.symbolRate = peakStatus.out;
            SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, peakStatus.tunerFreq, &acq_params));
            if (retCode != BERR_SUCCESS)
            {
               printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
               goto done;
            }

            /* check for user abort */
            while (1)
            {
               SATFE_OnIdle();
               if (SATFE_Platform_GetChar(false) > 0)
               {
                  printf("operation aborted by user!\n");
                  goto done;
               }

               /* wait for lock */
               SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
               if (status.bDemodLocked)
               {
                  peakStatus.tunerFreq += status.carrierError;
                  printf("LOCKED %s @ %d Hz, %d sym/sec, time=%u ms, snr=%.03f, carr_err=%d, sym_rate_err=%d\n",
                         SATFE_GetModeString(status.mode), peakStatus.tunerFreq, peakStatus.out, SATFE_Platform_GetTimerCount(), (float)(status.snrEstimate / 256.0),
                         status.carrierError, status.symbolRateError);
                  bLocked = true;
                  break;
               }
               else if (status.reacqCount > 2)
               {
                  printf("could not lock!\n");
                  break;
               }
               else
                  BKNI_Sleep(10);
            }
         }
         else
            bLocked = true;

         if (bSingleShot && bLocked)
            goto done;
      }

      determine_next_freq:
      if (bLocked)
      {
         if (abs(peakStatus.tunerFreq - last_found_freq) < (last_found_symbol_rate>>1))
         {
            /* this signal was previously found */
            printf("This signal was previously found (last_found_freq=%d, last_found_symbol_rate=%d)\n", last_found_freq, last_found_symbol_rate);
            last_found_freq = peakStatus.tunerFreq;
            curr_tuner_freq = last_freq + peak_scan_range;
         }
         else
         {
            nFound++;
            last_found_freq = peakStatus.tunerFreq;
            last_found_symbol_rate = peakStatus.out;
            f = (peakStatus.out*3/5); /* assume 20% nyquist rolloff */
            if (f < 2000000)
               f = 2000000; /* make sure there is at least 2MHz spacing between center frequencies of adjacent transponders */
            curr_tuner_freq = peakStatus.tunerFreq + f + peak_scan_range + peak_scan_carr_freq_step_size;
            if (last_freq > curr_tuner_freq)
               curr_tuner_freq = last_freq + peak_scan_range + peak_scan_carr_freq_step_size;
         }
      }
      else
         curr_tuner_freq = last_freq + peak_scan_range + peak_scan_carr_freq_step_size;
   }

   done:
   if (bVerbose)
   {
      printf("\nnumber of transponders found = %d\n", nFound);
      printf("total time = %d\n", SATFE_Platform_GetTimerCount());
   }
   SATFE_Platform_KillTimer();
   return true;
#else
   printf("This command is deprecated.  Use blind_scan_psd instead.\n");
   return true;
#endif
}


/******************************************************************************
 SATFE_Command_blind_scan_psd()
******************************************************************************/
bool SATFE_Command_blind_scan_psd(SATFE_Chip *pChip, int argc, char **argv)
{
#define TIMING_LOOP_TIMEOUT_MS 2000
   const uint32_t peak_scan_carr_freq_step_size = 250000;
   uint32_t peak_scan_range = 0;
   BAST_AcqSettings acq_params;
   BKNI_EventHandle peakScanEvent;
   BAST_ChannelStatus status;
   BERR_Code retCode;
   uint32_t min_symbol_rate, max_symbol_rate, min_freq = 0, max_freq = 0;
   bool bVerbose = true, bSingleShot = false, bDontAcquire = false, bCheckTimingLock = false;
   uint32_t curr_tuner_freq, last_freq, last_found_freq = 0, last_found_symbol_rate = 0, nFound = 0, search_range;
   uint32_t nCandidates = 0;
   BAST_PeakScanStatus peakStatus;
   int i;
   bool bLocked = false;
   float f;
   BAST_SignalDetectStatus signalDetectStatus;
   uint32_t nTimingLoopLocked = 0;
   uint8_t tuner_ctl;

   if (argc < 5)
   {
      SATFE_PrintDescription1("blind_scan_psd",
                              "blind_scan_psd [min_freq] [max_freq] [min_sym_rate] [max_sym_rate] <-q> <-a> <-l> <-t>",
                              "Scan and lock to a signal within a given frequency range.",
                              "min_freq = minimum RF frequency in the scan range", false);
      SATFE_PrintDescription2("max_freq = maximum RF frequency in the scan range.", false);
      SATFE_PrintDescription2("min_sym_rate = minimum symbol rate in sym/sec", false);
      SATFE_PrintDescription2("max_sym_rate = maximum symbol rate in sym/sec", false);
      SATFE_PrintDescription2("-q = suppress debug output", false);
      SATFE_PrintDescription2("-a = exit on first transponder found", false);
      SATFE_PrintDescription2("-l = do not acquire potential signal found by symbol rate scan", false);
      SATFE_PrintDescription2("-t = always check for timing loop lock before acquiring", true);
      return true;
   }

   /* process command line arguments */
   if (SATFE_GetFreqFromString(pChip, argv[1], &min_freq) == false)
      return false;
   if (SATFE_GetFreqFromString(pChip, argv[2], &max_freq) == false)
      return false;
   f = (float)atof(argv[3]);
   if (f < 50)
      min_symbol_rate = (uint32_t)(f * 1000000);
   else
      min_symbol_rate = (uint32_t)f;
   if ((min_symbol_rate < 256000UL) || (min_symbol_rate > 45000000UL))
   {
      printf("min_symbol_rate is out of range\n");
      return false;
   }
   f = (float)atof(argv[4]);
   if (f < 50)
      max_symbol_rate = (uint32_t)(f * 1000000);
   else
      max_symbol_rate = (uint32_t)f;
   if ((max_symbol_rate < 256000UL) || (max_symbol_rate > 45000000UL) || (max_symbol_rate <= min_symbol_rate))
   {
      printf("max_symbol_rate is out of range\n");
      return false;
   }

   for (i = 5; i < argc; i++)
   {
      if (!strcmp(argv[i], "-q"))
         bVerbose = false;
      else if (!strcmp(argv[i], "-a"))
         bSingleShot = true;
      else if (!strcmp(argv[i], "-l"))
         bDontAcquire = true;
      else if (!strcmp(argv[i], "-t"))
         bCheckTimingLock = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   if (bVerbose)
   {
      printf("sweep freq from %u to %u\n", min_freq, max_freq);
      printf("min symbol rate = %u, max symbol rate = %u\n", min_symbol_rate, max_symbol_rate);
   }

   nPsdCandidates = 0;

   /* start the timer to time the blind scan function */
   SATFE_Platform_StartTimer();

   /* bypass dft freq estimate search in acquisition */
   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig() error 0x%X\n", retCode);
      return retCode;
   }
   tuner_ctl |= BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST;
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_WriteConfig() error 0x%X\n", retCode);
      return retCode;
   }

   /* obtain the peak scan done event handle */
   BAST_GetPeakScanEventHandle(pChip->hAstChannel[pChip->currChannel], &peakScanEvent);

   /* initial range is half the full range */
   peak_scan_range = (max_freq - min_freq) >> 1;

   for (curr_tuner_freq = min_freq + peak_scan_range; curr_tuner_freq < (max_freq + peak_scan_range - (4*peak_scan_carr_freq_step_size)); )
   {
      /* check if user wants to abort */
      SATFE_OnIdle();
      if (SATFE_Platform_GetChar(false) > 0)
      {
         printf("operation aborted by user!\n");
         break;
      }

      /* find the next transponder starting from curr_tuner_freq */
      retCode = SATFE_ScanSymbolRatePsd(pChip, curr_tuner_freq, peak_scan_range, peak_scan_carr_freq_step_size, min_symbol_rate, max_symbol_rate, &peakStatus, &last_freq, bVerbose);
      if (retCode != BERR_SUCCESS)
         return false;
printf("SATFE_ScanSymbolRatePsd() returned: last_freq=%u, peakStatus.tunerFreq=%u, Fb=%u\n", last_freq, peakStatus.tunerFreq, peakStatus.out);
      bLocked = false;
      if (peakStatus.tunerFreq > 0)
      {
         /* potential transponder was found */
         nCandidates++;
         printf(">>> potential signal found at %d Hz (%d sym/s)\n", peakStatus.tunerFreq, peakStatus.out);

         if (bCheckTimingLock)
         {
            /* check if timing loop can lock */
            acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT | BAST_ACQSETTINGS_SIGNAL_DETECT_MODE;
            acq_params.carrierFreqOffset = 0;
            acq_params.mode = BAST_Mode_eLdpc_8psk_2_3; /* not used */
            acq_params.symbolRate = peakStatus.out;
            SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, peakStatus.tunerFreq, &acq_params));
            if (retCode != BERR_SUCCESS)
            {
               printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
               goto done;
            }

            /* wait for signal detection done */
            for (i = TIMING_LOOP_TIMEOUT_MS; i; i--)
            {
               SATFE_MUTEX(retCode = BAST_GetSignalDetectStatus(pChip->hAstChannel[pChip->currChannel], &signalDetectStatus));
               if (retCode != BERR_SUCCESS)
               {
                  printf("BAST_GetSignalDetectStatus() error 0x%X\n", retCode);
                  goto done;
               }

               /* sanity check */
               if (signalDetectStatus.bEnabled == false)
               {
                  printf("BAST_GetSignalDetectStatus() error: bEnabled should not be false!\n");
                  goto done;
               }

               if (signalDetectStatus.bDone)
                  break;

               BKNI_Sleep(1);
            }

            if (i > 0)
            {
               if (!signalDetectStatus.bTimingLoopLocked)
               {
                  printf("Unable to lock timing loop\n");
                  goto determine_next_freq;
               }
               else
               {
                  nTimingLoopLocked++;
                  printf("Timing loop is locked (%d ms)\n", TIMING_LOOP_TIMEOUT_MS-i);
               }
            }
            else
               printf("ERROR: signal detect function timeout\n");
         }

         if (!bDontAcquire)
         {
            /* limit search range for low symbol rates */
            if (peakStatus.out > 8000000)
               search_range = 5000000;
            else
            {
               search_range = peakStatus.out / 2;
               if (search_range > 3000000)
                  search_range = 3000000;
               else if (search_range < 1000000)
                  search_range = 1000000;
            }

            SATFE_MUTEX(retCode = BAST_SetSearchRange(pChip->hAst, search_range));
            if (retCode != BERR_SUCCESS)
            {
               printf("BAST_SetSearchRange() error 0x%X\n", retCode);
               goto done;
            }

            /* do a blind acquisition on this signal given {freq,symbol_rate} from the peak scan */
            acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
            acq_params.carrierFreqOffset = 0;
            acq_params.mode = BAST_Mode_eBlindScan;
            acq_params.symbolRate = peakStatus.out;
            SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, peakStatus.tunerFreq, &acq_params));
            if (retCode != BERR_SUCCESS)
            {
               printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
               goto done;
            }

            /* check for user abort */
            while (1)
            {
               SATFE_OnIdle();
               if (SATFE_Platform_GetChar(false) > 0)
               {
                  printf("operation aborted by user!\n");
                  goto done;
               }

               /* wait for lock */
               SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
               if (status.bDemodLocked)
               {
                  peakStatus.tunerFreq += status.carrierError;
                  printf(">>> LOCKED %s @ %d Hz, %d sym/sec, time=%u ms, snr=%.03f, carr_err=%d, sym_rate_err=%d\n",
                         SATFE_GetModeString(status.mode), peakStatus.tunerFreq, peakStatus.out, SATFE_Platform_GetTimerCount(), (float)(status.snrEstimate / 256.0),
                         status.carrierError, status.symbolRateError);
                  bLocked = true;
                  break;
               }
               else if (status.reacqCount > 2)
               {
                  printf(">>> could not lock %u sym/sec at %u Hz\n", peakStatus.out, peakStatus.tunerFreq);
                  break;
               }
               else
                  BKNI_Sleep(10);
            }
         }
         else
            bLocked = true;

         if (bSingleShot && bLocked)
            goto done;
      }
      else
         printf(">>> no signal found from symbol rate scan\n");

      determine_next_freq:
      if (bLocked)
      {
         if (abs(peakStatus.tunerFreq - last_found_freq) < (last_found_symbol_rate>>1))
         {
            /* this signal was previously found */
            printf("This signal was previously found (last_found_freq=%d, last_found_symbol_rate=%d)\n", last_found_freq, last_found_symbol_rate);
            last_found_freq = peakStatus.tunerFreq;
            goto continue_from_last_freq;
         }
         else
         {
            nFound++;
            last_found_freq = peakStatus.tunerFreq;
            last_found_symbol_rate = peakStatus.out;
            f = (peakStatus.out*3/5); /* assume 20% nyquist rolloff */
            if (f < 2000000)
               f = 2000000; /* make sure there is at least 2MHz spacing between center frequencies of adjacent transponders */
            curr_tuner_freq = peakStatus.tunerFreq + f + peak_scan_carr_freq_step_size;
            if (last_freq > curr_tuner_freq)
               goto continue_from_last_freq;
            else
               goto update_peak_scan_range;
         }
      }
      else
      {
         continue_from_last_freq:
         curr_tuner_freq = last_freq + peak_scan_carr_freq_step_size;

         update_peak_scan_range:
         curr_tuner_freq -= 750000; /* this is to ensure we see the rising edge of a potential adjacent signal */
         if (curr_tuner_freq < max_freq)
         {
            peak_scan_range = (max_freq - curr_tuner_freq) >> 1;
            printf("changing peak_scan_range to %u\n", peak_scan_range);
         }
         curr_tuner_freq += peak_scan_range;
      }
   }

   done:
   /* restore dft search in acquisition */
   tuner_ctl &= ~BAST_G3_CONFIG_TUNER_CTL_BYPASS_DFT_FREQ_EST;
   SATFE_MUTEX(retCode = BAST_WriteConfig(pChip->hAstChannel[pChip->currChannel], BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL));
   printf("\ntotal time = %d\n", SATFE_Platform_GetTimerCount());
   printf("number of PSD candidates = %d\n", nPsdCandidates);
   printf("number of TP candidates = %d\n", nCandidates);
   if (bCheckTimingLock)
      printf("number of TP candidates that locked timing loop = %d\n", nTimingLoopLocked);
   printf("number of TP candidates that locked demod = %d\n", nFound);
   SATFE_Platform_KillTimer();
   return true;
}


/******************************************************************************
 SATFE_Command_lock_transponders()
******************************************************************************/
bool SATFE_Command_lock_transponders(SATFE_Chip *pChip, int argc, char **argv)
{
   typedef struct
   {
      uint16_t freqMhz;
      uint16_t baudKhz;
   } transponder_t;

   static transponder_t transponder_list[] = {
      {1156,12416},
      {1166,2170},
      {1176,7500},
      {1185,7500},
      {1191,1875},
      {1195,4400},
      {1203,7200},
      {1454,3600},
      {1461,7500},
      {1470,7500},
      {1479,7500},
      {1486,3817},
      {1494,3392},
      {1500,4999},
      {1504,2400},
      {1509,4583},
      {1518,4687},
      {1703,7142},
      {1753,2300},
      {1760,7400},
      {1767,4573},
      {1772,3616},
      {1777,3999},
      {1785,4069},
      {1837,7500},
      {1846,7500},
      {1855,7500},
      {1864,7500},
      {1876,7500},
      {1917,7500},
      {1926,7500},
      {1935,7500},
      {1944,7500},
      {1997,6222},
      {2035,4999},
      {2050,15000},
      {2061,2220},
      {2065,4999},
      {2077,6665},
      {2083,2499},
      {2086,2979},
      {2094,3931},
      {2099,2777},
      {2102,2170},
      {2106,3213},
      {0, 0}
   };

   int i;
   uint32_t freq, Fb, search_range, nTransponders = 0, nLocked = 0, t;
   float input_power;
   BERR_Code retCode;
   BAST_AcqSettings acq_params;
   BAST_ChannelStatus status;

   if (argc != 1)
   {
      SATFE_PrintDescription1("lock_transponders", "lock_transponders", "Try to lock a list of transponders.", "none", true);
      return true;
   }

   for (i = 0; transponder_list[i].freqMhz != 0; i++)
   {
      nTransponders++;
      printf("#%d: %u MHz, %u Ksps -> ", nTransponders, transponder_list[i].freqMhz, transponder_list[i].baudKhz);
      freq = transponder_list[i].freqMhz * 1000000;
      Fb = transponder_list[i].baudKhz * 1000;

      SATFE_Platform_StartTimer();

      /* limit search range for low symbol rates */
      if (Fb > 8000000)
         search_range = 5000000;
      else
      {
         search_range = Fb / 2;
         if (search_range > 3000000)
            search_range = 3000000;
         else if (search_range < 1000000)
            search_range = 1000000;
      }
      SATFE_MUTEX(retCode = BAST_SetSearchRange(pChip->hAst, search_range));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_SetSearchRange() error 0x%X\n", retCode);
         goto done;
      }

      /* do a blind acquisition on this signal given {freq,symbol_rate} from the peak scan */
      acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT;
      acq_params.carrierFreqOffset = 0;
      acq_params.mode = BAST_Mode_eBlindScan;
      acq_params.symbolRate = Fb;
      SATFE_MUTEX(retCode = SATFE_TuneAcquire(pChip, freq, &acq_params));
      if (retCode != BERR_SUCCESS)
      {
         printf("SATFE_TuneAcquire() error 0x%X\n", retCode);
         goto done;
      }

      /* check for user abort */
      while (1)
      {
         SATFE_OnIdle();
         if (SATFE_Platform_GetChar(false) > 0)
         {
            printf("operation aborted by user!\n");
            goto done;
         }

         /* wait for lock */
         SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
         if (status.bDemodLocked)
         {
            t = SATFE_Platform_GetTimerCount();
            BKNI_Sleep(1000);
            SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
            if (status.bDemodLocked)
            {
               SATFE_Platform_GetInputPower(pChip, status.RFagc, status.IFagc, (uint32_t)status.agf, status.tunerFreq, &input_power);
               printf("LOCKED %s, t=%u ms, snr=%.03f, carr_err=%d, Fb_err=%d, power=%.03f, reacqs=%d\n",
                      SATFE_GetModeString(status.mode), t, (float)(status.snrEstimate / 256.0),
                      status.carrierError, status.symbolRateError, input_power, status.reacqCount);
               nLocked++;
               break;
            }
         }
         else if (status.reacqCount > 2)
         {
            printf("NOT LOCKED, t=%u\n", SATFE_Platform_GetTimerCount());
            break;
         }
         else
            BKNI_Sleep(1);
      }
   }

   done:
   printf("Number of transponders locked = %u\n", nLocked);
   return true;
}


/******************************************************************************
 SATFE_Command_config_diseqc()
******************************************************************************/
bool SATFE_Command_config_diseqc(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bDone = false;
   BAST_DiseqcSettings settings;
   char c, str[16];

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("config_diseqc", "config_diseqc", "Configure diseqc settings.", "none", true);
      return true;
   }

   while (!bDone)
   {
      SATFE_MUTEX(retCode = BAST_GetDiseqcSettings(pChip->hAstChannel[pChip->currChannel], &settings));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetDiseqcSettings() error 0x%X\n", retCode);
         return false;
      }

      printf("Select diseqc parameter to change:\n");
      printf("a) TXOUT pin mode                     : %s\n", settings.bEnvelope ? "envelope" : "tone");
      printf("b) tone alignment                     : %s\n", settings.bToneAlign ? "on" : "off");
      printf("c) rx reply time out                  : %s\n", settings.bDisableRRTO ? "disabled" : "enabled");
      printf("d) tone burst                         : %s\n", settings.bEnableToneburst ? "enabled" : "disabled");
      if (settings.bEnableToneburst)
      {
         printf("e) tone burst mode                    : %s\n", settings.bToneburstB ? "Tone B" : "Tone A");
      }
      printf("f) reply mode                         : %s\n", settings.bOverrideFraming ? "manual" : "determined by framing byte");
      if (settings.bOverrideFraming)
         printf("g) expect reply                       : %s\n", settings.bExpectReply ? "yes" : "no");
      printf("h) LNBPU                              : %s\n", settings.bEnableLNBPU ? "on TXEN pin" : "not used");
      printf("i) disable rx only                    : %s\n", settings.bDisableRxOnly ? "true" : "false");
      printf("j) RRTO (usecs)                       : %u\n", settings.rrtoUsec);
      printf("k) rx bit detection threshold (usecs) : %u\n", settings.bitThreshold);
      printf("l) tone threshold (.16 counts/mV)     : %u\n", settings.toneThreshold);
      printf("m) pre-tx delay (msecs)               : %u\n", settings.preTxDelay);
      printf("n) overvoltage threshold              : %u\n", settings.vsenseThresholdHi);
      printf("o) undervoltage threshold             : %u\n", settings.vsenseThresholdLo);
      printf("x) exit\n");
      printf("Selection: ");
      c = SATFE_Platform_GetChar(true);
      fflush(stdin);
      printf("\n\n");
      switch (c)
      {
         case 'a':
            settings.bEnvelope = settings.bEnvelope ? false : true;
            break;
         case 'b':
            settings.bToneAlign = settings.bToneAlign ? false : true;
            break;
         case 'c':
            settings.bDisableRRTO = settings.bDisableRRTO ? false : true;
            break;
         case 'd':
            settings.bEnableToneburst = settings.bEnableToneburst ? false : true;
            break;
         case 'e':
            settings.bToneburstB = settings.bToneburstB ? false : true;
            break;
         case 'f':
            settings.bOverrideFraming = settings.bOverrideFraming ? false : true;
            break;
         case 'g':
            settings.bExpectReply = settings.bExpectReply ? false : true;
            break;
         case 'h':
            settings.bEnableLNBPU = settings.bEnableLNBPU ? false : true;
            break;
         case 'i':
            settings.bDisableRxOnly = settings.bDisableRxOnly ? false : true;
            break;
         case 'j':
            SATFE_GetString(str);
            settings.rrtoUsec = strtoul(str, NULL, 10);
            break;
         case 'k':
            SATFE_GetString(str);
            settings.bitThreshold = (uint16_t)atoi(str);
            break;
         case 'l':
            SATFE_GetString(str);
            settings.toneThreshold = (uint8_t)atoi(str);
            break;
         case 'm':
            SATFE_GetString(str);
            settings.preTxDelay = (uint8_t)atoi(str);
            break;
         case 'n':
            SATFE_GetString(str);
            settings.vsenseThresholdHi = (uint8_t)atoi(str);
            break;
         case 'o':
            SATFE_GetString(str);
            settings.vsenseThresholdLo = (uint8_t)atoi(str);
            break;
         case 'x':
            bDone = true;
            break;
         default:
            printf("Invalid selection!\n\n");
            break;
      }

      if (!bDone)
      {
         SATFE_MUTEX(retCode = BAST_SetDiseqcSettings(pChip->hAstChannel[pChip->currChannel], &settings));
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_SetDiseqcSettings() error 0x%X\n", retCode);
            return false;
         }
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_xport()
******************************************************************************/
bool SATFE_Command_xport(SATFE_Chip *pChip, int argc, char **argv)
{
   static char* BCH_MPEG_ERROR_MODE_STR[] =
   {
      "BCH or CRC8",
      "CRC8",
      "BCH",
      "BCH and CRC8"
   };

   BERR_Code retCode;
   bool bDone = false;
   BAST_OutputTransportSettings settings;
   int i;
   char c;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("xport",
                              "xport",
                              "Configure output transport settings.",
                              "none", true);
      return true;
   }

   while (!bDone)
   {
      SATFE_MUTEX(retCode = BAST_GetOutputTransportSettings(pChip->hAstChannel[pChip->currChannel], &settings));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetOutputTransportSettings() error 0x%X\n", retCode);
         return false;
      }

      printf("Select transport parameter to change:\n");
      printf("a) output mode     = %s\n", settings.bSerial ? "serial" : "parallel");
      printf("b) clk polarity    = %s\n", settings.bClkInv ? "inverted" : "normal");
      printf("c) clk suppression = %s\n", settings.bClkSup ? "true" : "false");
      printf("d) vld polarity    = %s\n", settings.bVldInv ? "inverted" : "normal");
      printf("e) sync polarity   = %s\n", settings.bSyncInv ? "inverted" : "normal");
      printf("f) err polarity    = %s\n", settings.bErrInv ? "inverted" : "normal");
      printf("g) xbert           = %s\n", settings.bXbert ? "true" : "false");
      printf("h) tei             = %s\n", settings.bTei ? "on" : "off");
      printf("i) delay           = %s\n", settings.bDelay ? "true" : "false");
      printf("j) sync1           = %s\n", settings.bSync1 ? "true" : "false");
      printf("k) head4           = %s\n", settings.bHead4 ? "true" : "false");
      printf("l) del header      = %s\n", settings.bDelHeader ? "true" : "false");
      printf("m) bypass opll     = %s\n", settings.bOpllBypass ? "true" : "false");
      printf("n) bch mpeg error  = %s\n", BCH_MPEG_ERROR_MODE_STR[settings.bchMpegErrorMode]);
      printf("x) exit\n");
      printf("Selection: ");
      c = SATFE_Platform_GetChar(true);
      fflush(stdin);
      printf("\n\n");

      switch (c)
      {
         case 'a':
            settings.bSerial = settings.bSerial ? false : true;
            break;
         case 'b':
            settings.bClkInv = settings.bClkInv ? false : true;
            break;
         case 'c':
            settings.bClkSup = settings.bClkSup ? false : true;
            break;
         case 'd':
            settings.bVldInv = settings.bVldInv ? false : true;
            break;
         case 'e':
            settings.bSyncInv = settings.bSyncInv ? false : true;
            break;
         case 'f':
            settings.bErrInv = settings.bErrInv ? false : true;
            break;
         case 'g':
            settings.bXbert = settings.bXbert ? false : true;
            break;
         case 'h':
            settings.bTei = settings.bTei ? false : true;
            break;
         case 'i':
            settings.bDelay = settings.bDelay ? false : true;
            break;
         case 'j':
            settings.bSync1 = settings.bSync1 ? false : true;
            break;
         case 'k':
            settings.bHead4 = settings.bHead4 ? false : true;
            break;
         case 'l':
            settings.bDelHeader = settings.bDelHeader ? false : true;
            break;
         case 'm':
            settings.bOpllBypass = settings.bOpllBypass ? false : true;
            break;
         case 'n':
            choose_error_mode:
            printf("Select BCH MPEG Error Mode:\n");
            for (i = 0; i < 4; i++)
               printf("%d) %s\n", i, BCH_MPEG_ERROR_MODE_STR[i]);
            printf("x) exit\n");
            printf("Selection: ");
            c = SATFE_Platform_GetChar(true);
            fflush(stdin);
            printf("\n\n");
            if ((c >= '0') || (c <= '3'))
               settings.bchMpegErrorMode = (BAST_BchMpegErrorMode)(c - '0');
            else if (c != 'x')
            {
               printf("Invalid selection!\n\n");
               goto choose_error_mode;
            }
            break;
         case 'x':
            bDone = true;
            break;
         default:
            printf("Invalid selection!\n\n");
            break;
      }

      if (!bDone)
      {
         SATFE_MUTEX(retCode = BAST_SetOutputTransportSettings(pChip->hAstChannel[pChip->currChannel], &settings));
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_SetOutputTransportSettings() error 0x%X\n", retCode);
            return false;
         }
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_check_timing_lock()
******************************************************************************/
bool SATFE_Command_check_timing_lock(SATFE_Chip *pChip, int argc, char **argv)
{
   BAST_SignalDetectStatus status;
   BAST_AcqSettings acq_params;
   BERR_Code retCode;
   uint32_t freq, timeout;

   if (argc != 2)
   {
      SATFE_PrintDescription1("check_timing_lock",
                              "check_timing_lock [tuner_freq]",
                              "Attempt to lock the timing loop at the specified tuner_freq",
                              "tuner_freq = RF frequency", true);
      return true;
   }

   if (SATFE_GetFreqFromString(pChip, argv[1], &freq) == false)
      return false;

   acq_params.acq_ctl = BAST_ACQSETTINGS_DEFAULT | BAST_ACQSETTINGS_SIGNAL_DETECT_MODE;;
   acq_params.carrierFreqOffset = 0;
   acq_params.mode = BAST_Mode_eLdpc_8psk_2_3; /* mode doesn't matter for the signal detect function */
   acq_params.symbolRate = pChip->acqSettings[pChip->currAcqSettings].symbolRate;
   retCode = SATFE_TuneAcquire(pChip, freq, &acq_params);
   SATFE_GOTO_DONE_ON_ERROR("SATFE_TuneAcquire()", retCode);

   for (timeout = 2000; timeout > 0; timeout--)
   {
      SATFE_MUTEX(retCode = BAST_GetSignalDetectStatus(pChip->hAstChannel[pChip->currChannel], &status));
      SATFE_GOTO_DONE_ON_ERROR("BAST_GetSignalDetectStatus()", retCode);

      if (status.bDone)
         break;
      if (status.bEnabled == 0)
      {
         printf("ERROR: signal detect status not enabled!\n"); /* should never get here */
         goto done;
      }

      SATFE_OnIdle();
      BKNI_Sleep(1);
   }
   if (timeout == 0)
      printf("timeout error!\n");
   else if (status.bTimingLoopLocked)
      printf("timing loop is locked\n");
   else
      printf("timing loop is not locked\n");

   done:
   SATFE_RETURN_ERROR("SATFE_Command_check_timing_lock()", retCode);
}


/******************************************************************************
 SATFE_Command_search_tone()
******************************************************************************/
bool SATFE_Command_search_tone(SATFE_Chip *pChip, int argc, char **argv)
{
   BAST_ChannelStatus status;
   bool bRatio = false, bCont = false;
   uint32_t start_freq, end_freq, center_freq, range;
   int i;
   float f;
   uint32_t attempts = 0, num_tone_detect = 0, tuner_freq, min_pow, max_pow, max_pow_freq, max_pow_idx, freq_step, dft_size, Fs;
   BAST_PeakScanStatus peakStatus;
   BKNI_EventHandle peakScanEvent;
   BERR_Code retCode;
#ifdef SATFE_USE_FLOAT
   float binsize, power_detect_ratio = 2.5, min_ratio = 999999.0, max_ratio = 0.0, ratio;
#else
   uint32_t binsize; /* 2^8 scale */
   uint32_t power_detect_ratio = 20; /* 2^3 scale, 2.5*8=20 */
   uint32_t min_ratio = 0x7FFFFFFF, max_ratio = 0, ratio; /* 2^2 scale */
   uint32_t P_hi, P_lo, Q_hi, Q_lo, threshold;
#endif

   if (argc < 4)
   {
      SATFE_PrintDescription1("search_tone",
                              "search_tone [freq] [range] [step] <-r [ratio]> <-t>",
                              "Find the center frequency of an unmodulated carrier.",
                              "freq = RF frequency", false);
      SATFE_PrintDescription2("range = search range", false);
      SATFE_PrintDescription2("step = step size in Hz", false);
      SATFE_PrintDescription2("ratio = minimum ratio of max power to min power for tone detection (default is 2.5)", false);
      SATFE_PrintDescription2("-t = search tone continuously until user aborts test", true);
      return true;
   }

   if (SATFE_GetFreqFromString(pChip, argv[1], &center_freq) == false)
      return false;

   f = (float)atof(argv[2]);
   if (f < 200)
      range = (uint32_t)(f * 1000000);
   else
      range = (uint32_t)f;

   start_freq = center_freq - range;
   end_freq = center_freq + range;

   f = (float)atof(argv[3]);
   if (f < 10)
      freq_step = (uint32_t)(f * 1000000);
   else
      freq_step = (uint32_t)f;

   for (i = 4; i < argc; i++)
   {
      if (bRatio)
      {
         bRatio = false;
#ifdef SATFE_USE_FLOAT
         power_detect_ratio = (float)atof(argv[i]);
#else
         power_detect_ratio = (uint32_t)(atof(argv[i]) * 8);
#endif
      }
      else if (!strcmp(argv[i], "-t"))
         bCont = true;
      else if (!strcmp(argv[i], "-r"))
         bRatio = true;
      else
      {
         printf("unrecognized parameter (%s)\n", argv[i]);
         return false;
      }
   }

   /* set peak_scan_sym_rate_min and peak_scan_sym_rate_max to 0 for tone detection */
   SATFE_MUTEX(retCode = BAST_SetPeakScanSymbolRateRange(pChip->hAstChannel[pChip->currChannel], 0, 0));
   SATFE_GOTO_DONE_ON_ERROR("BAST_SetPeakScanSymbolRateRange()", retCode);

   /* obtain the peak scan done event handle */
   BAST_GetPeakScanEventHandle(pChip->hAstChannel[pChip->currChannel], &peakScanEvent);

   /* get the sample freq */
   SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
   SATFE_GOTO_DONE_ON_ERROR("BAST_GetChannelStatus()", retCode);
   Fs = status.sample_clock;
   dft_size = 512;
#ifdef SATFE_USE_FLOAT
   binsize = (float)((float)Fs / ((float)dft_size * 64.0)); /* Fs/(DFT_size*2^HB), HB=6 */
   //freq_step = (uint32_t)(binsize * 256.0);
#else
   BAST_MultU32U32(Fs, 512, &P_hi, &P_lo);
   BAST_DivU64U32(P_hi, P_lo, dft_size * 64, &Q_hi, &Q_lo);
   binsize = (Q_lo+1)>>1; /* binsize is scaled 2^8 */
   //freq_step = binsize;
#endif

   max_pow = 0;
   min_pow = 0xFFFFFFFF;
   max_pow_freq = start_freq;
   max_pow_idx = 0;

   printf("search for tone from %d Hz to %d Hz in steps of %d Hz\n", start_freq, end_freq, freq_step);
   printf("power_detect_ratio = %f\n", power_detect_ratio/8.0);
   printf("Fs = %d Hz, binsize = %d\n", Fs, binsize);

   /* sweep from start_freq to end_freq and record min and max power */
   do_peak_scan:
   /* start the timer to time the blind scan function */
   SATFE_Platform_StartTimer();

   for (tuner_freq = start_freq; tuner_freq <= end_freq; tuner_freq += freq_step)
   {
      /* clear the peak scan done event */
      BKNI_WaitForEvent(peakScanEvent, 0);

      /* initiate peak scan */
      SATFE_MUTEX(retCode = BAST_PeakScan(pChip->hAstChannel[pChip->currChannel], tuner_freq));
      SATFE_GOTO_DONE_ON_ERROR("BAST_PeakScan()", retCode);

      /* wait for peak scan done */
#if defined(DIAGS) || defined(WIN32)
      for (i = 0; i < 500; i++)
      {
         retCode = BKNI_WaitForEvent(peakScanEvent, 0);
         if (retCode == BERR_SUCCESS)
            break;
         SATFE_OnIdle();
         BKNI_Sleep(1);
      }
#else
      retCode = BKNI_WaitForEvent(peakScanEvent, 500);
#endif
      if (retCode != BERR_SUCCESS)
      {
         printf("peak scan event not received!\n");
         return false;
      }

      /* get the results of the peak scan */
      SATFE_MUTEX(retCode = BAST_GetPeakScanStatus(pChip->hAstChannel[pChip->currChannel], &peakStatus));
      SATFE_GOTO_DONE_ON_ERROR("BAST_GetPeakScanStatus()", retCode);

      /* verify that the peak scan finished successfully */
      if (peakStatus.status != 0)
      {
         printf("ERROR: peak scan status = %d\n", peakStatus.status);
         return false;
      }

      printf("%d Hz: idx=%04X, pow=0x%08X(%d)\n", peakStatus.tunerFreq, peakStatus.out, peakStatus.peakPower, peakStatus.peakPower);

      /* keep track of min and max power over the frequency range */
      if (peakStatus.peakPower > max_pow)
      {
         max_pow = peakStatus.peakPower;
         max_pow_freq = peakStatus.tunerFreq;
         max_pow_idx = peakStatus.out;
      }
      if (peakStatus.peakPower < min_pow)
         min_pow = peakStatus.peakPower;

      if (freq_step == 0)
         break;
   }

   /* choose freq with highest power */
#ifdef SATFE_USE_FLOAT
   ratio = (float)((double)max_pow / (double)min_pow);
#else
   BAST_MultU32U32(max_pow, 16, &P_hi, &P_lo);
   BAST_DivU64U32(P_hi, P_lo, min_pow, &Q_hi, &Q_lo);
   ratio = (Q_lo+1)>>1; /* scaled 2^3 */
#endif
   if (ratio < min_ratio)
      min_ratio = ratio;
   if (ratio > max_ratio)
      max_ratio = ratio;

#ifdef SATFE_USE_FLOAT
   printf("min=%d, max=%d, ratio=%f\n", min_pow, max_pow, ratio);
#else
   printf("min=%u, max=%u, ratio*4=%u\n", min_pow, max_pow, ratio);
#endif

   attempts++;
#ifdef SATFE_USE_FLOAT
   if (max_pow > (min_pow * power_detect_ratio))
#else
   threshold = (min_pow * power_detect_ratio);
   if (max_pow > threshold)
#endif
   {
#ifdef SATFE_USE_FLOAT
      max_pow_freq += (uint32_t)((float)max_pow_idx * binsize);
#else
      BAST_MultU32U32(max_pow_idx, binsize, &P_hi, &P_lo);
      BAST_DivU64U32(P_hi, P_lo, 256, &Q_hi, &Q_lo);
      max_pow_freq += Q_lo;
#endif
      num_tone_detect++;
      printf("tone detected at %u Hz\n", max_pow_freq);
   }
   else
      printf("no tone detected\n");
   printf("t = %d\n", SATFE_Platform_GetTimerCount());

   if (bCont)
   {
#ifdef SATFE_USE_FLOAT
      printf("%d/%d: min=%f, max=%f, curr=%f\n", num_tone_detect, attempts, min_ratio, max_ratio, ratio);
#else
      printf("%d/%d: min=%f, max=%f, curr=%f\n", num_tone_detect, attempts, (float)min_ratio/4.0, (float)max_ratio/4.0, (float)ratio/4.0);
#endif
      min_ratio = 999999.0;
      max_ratio = 0.0;
      if (SATFE_WaitForKeyPress(1) <= 0)
         goto do_peak_scan;
   }

   done:
   SATFE_Platform_KillTimer();
   SATFE_RETURN_ERROR("SATFE_Command_search_tone()", retCode);
}


/******************************************************************************
 SATFE_Command_power_down()
******************************************************************************/
bool SATFE_Command_power_down(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t options;

   if (argc != 2)
   {
      SATFE_PrintDescription1("power_down", "power_down [options]",
                              "power down downstream cores.",
                              "options = {bit 0: TUNER/SDS/FEC, bit 1: DISEQC, bit 2: channelizer}", true);
      return true;
   }

   options = strtoul(argv[1], NULL, 16);

   retCode = BAST_PowerDown(pChip->hAstChannel[pChip->currChannel], options);
   SATFE_RETURN_ERROR("SATFE_Command_power_down()", retCode);
}


/******************************************************************************
 SATFE_Command_power_up()
******************************************************************************/
bool SATFE_Command_power_up(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t options;

   if (argc != 2)
   {
      SATFE_PrintDescription1("power_up", "power_up [options]",
                              "power up downstream cores.",
                              "options = {bit 0: TUNER/SDS/FEC, bit 1: DISEQC, bit 2: channelizer}", true);
      return true;
   }

   options = strtoul(argv[1], NULL, 16);

   retCode = BAST_PowerUp(pChip->hAstChannel[pChip->currChannel], options);
   if (pChip->chip.platformFunctTable->Configure != NULL)
   {
      /* setup platform configurations */
      pChip->chip.platformFunctTable->Configure((struct SATFE_Chip*)pChip, NULL);
   }

   SATFE_RETURN_ERROR("SATFE_Command_power_up()", retCode);
}


/******************************************************************************
 SATFE_Command_test_power_down()
******************************************************************************/
bool SATFE_Command_test_power_down(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t options = BAST_CORE_ALL, freq = 0;
   uint32_t count = 0;
   BKNI_EventHandle hLockEvent;
   bool bLocked;

   BSTD_UNUSED(argv);

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("test_power_down", "test_power_down <freq>",
                              "Test power down and up repeatedly.",
                              "freq = optionally acquire to this freq before and after power down/up", true);
      return true;
   }

   if (argc == 2)
   {
      if (SATFE_GetFreqFromString(pChip, argv[1], &freq) == false)
         return false;

      BAST_GetLockStateChangeEventHandle(pChip->hAstChannel[pChip->currChannel], &hLockEvent);

      printf("Acquiring...");
      if (SATFE_TuneAcquire(pChip, freq, NULL) != BERR_SUCCESS)
         return false;

      BKNI_WaitForEvent(hLockEvent, 1000);
      SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked));
      printf("%s\n", bLocked ? "locked" : "not locked");
   }

   while (SATFE_Platform_GetChar(false) <= 0)
   {
      retCode = BAST_PowerDown(pChip->hAstChannel[pChip->currChannel], options);
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_PowerDown() error 0x%02X\n", retCode);
         return false;
      }
      printf("AST power down...\n");

      BKNI_Sleep(5000);

      retCode = BAST_PowerUp(pChip->hAstChannel[pChip->currChannel], options);
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_PowerUp() error 0x%02X\n", retCode);
         return false;
      }
      printf("AST power up (%d)\n", count++);

      if (freq > 0)
      {
         printf("Acquiring...");
         if (SATFE_TuneAcquire(pChip, freq, NULL) != BERR_SUCCESS)
            return false;
         retCode = BKNI_WaitForEvent(hLockEvent, 0); /* remove not locked event */

         /* wait for initial lock */
         retCode = BKNI_WaitForEvent(hLockEvent, 1000);
         if (retCode == BERR_SUCCESS)
         {
            while (BKNI_WaitForEvent(hLockEvent, 30) == BERR_SUCCESS); /* wait for lock stable */
         }
         SATFE_MUTEX(retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked));
         printf("%s\n", bLocked ? "locked" : "not locked");
         if (bLocked == false)
         {
            printf("aborting test due to demod not locked\n");
            break;
         }
      }
      else
         BKNI_Sleep(2000);
   }

   SATFE_RETURN_ERROR("SATFE_Command_test_power_down()", retCode);
}


/******************************************************************************
 SATFE_Command_freq_sweep()
******************************************************************************/
bool SATFE_Command_freq_sweep(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_AcqSettings *pAcqSettings;
   uint32_t currentFreq, freqStart, freqStop, freqStep;
   float f;

   pAcqSettings = &(pChip->acqSettings[pChip->currAcqSettings]);

   freqStart = 950000000UL;
   freqStop = 2150000000UL;
   freqStep = 5000000;
   currentFreq = freqStart;

   if ((argc < 3) || (argc > 4))
   {
      SATFE_PrintDescription1("freq_sweep", "freq_sweep [freqStart] [freqStop] <step>",
                              "Sweep tuner frequency.",
                              "freqStart = start frequency", false);
      SATFE_PrintDescription2("freqStop = stop frequency", false);
      SATFE_PrintDescription2("step = (optional, default=5MHz) frequency step in MHz", true);
      return true;
   }

   if (SATFE_GetFreqFromString(pChip, argv[1], &freqStart) == false)
      return false;
   if (SATFE_GetFreqFromString(pChip, argv[2], &freqStop) == false)
      return false;

   if (argc == 4)
   {
      f = (float)atof(argv[3]);
      freqStep = (uint32_t)(f * 1000000);
   }

   if (freqStop <= freqStart)
   {
      printf("freqStop must be greater than freqStart!\n");
      return false;
   }

   while (SATFE_Platform_GetChar(false) <= 0)
   {
      printf("Tuning to %dHz\n", currentFreq);
      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], currentFreq, pAcqSettings));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_TuneAcquire() error 0x%02X\n", retCode);
         return false;
      }

      BKNI_Sleep(5000);

      currentFreq += freqStep;
      if (currentFreq > freqStop)
         break;
   }

   SATFE_RETURN_ERROR("SATFE_Command_test_power_down()", retCode);
}


#ifndef SATFE_USE_BFSK
/******************************************************************************
 SATFE_Ftm_ResetUc()
******************************************************************************/
BERR_Code SATFE_Ftm_ResetUc(SATFE_Chip *pChip)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[3] = {0x82, 0x07, 0x00};

   /* reset the ftm uC */
   printf("\n*** Resetting FTM uC ***\n");
   SATFE_MUTEX(retCode = BAST_ResetFtm(pChip->hAst));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ResetFtm() error 0x%02X, trying local reset command...\n", retCode);
      if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, 3, buf, 3000, 1, 0x07)) != BERR_SUCCESS)
      {
         printf("Failed to get RFmicro reset message! error 0x%02X\n", retCode);
      }
   }
   if(BERR_SUCCESS == retCode)
      ResetStatus=true;
   return retCode;
}


/******************************************************************************
 SATFE_LogFtmWrite()
******************************************************************************/
void SATFE_Ftm_LogWrite(SATFE_Chip *pChip, uint8_t *buf, uint8_t len)
{
   uint8_t i;

   if (!(pChip->bEnableFtmLogging))
      return;

   if (len > 0)
   {
      printf("FTM write: (0x%02X) ", buf[0]);
      for (i = 1; i < len; i++)
      {
         printf("0x%02X ", buf[i]);
         if (((buf[0] & 0x80) == 0) && (i == 5))
            printf("| ");
      }
      printf("\n");
   }
}


/******************************************************************************
 SATFE_LogFtmRead()
******************************************************************************/
void SATFE_Ftm_LogRead(SATFE_Chip *pChip, uint8_t *buf, uint8_t len)
{
   uint8_t i;

   if (!(pChip->bEnableFtmLogging))
      return;

   if (len > 0)
   {
      printf("FTM rcvd [#%d]: (0x%02X) ", pChip->ftmPacketCount, buf[0]);
      for (i = 1; i < len; i++)
      {
         printf("0x%02X ", buf[i]);
         if (((buf[0] & 0x80) == 0) && (i == 5))
            printf("| ");
      }
      printf("\n");
   }
}


/******************************************************************************
 SATFE_Ftm_SendMessage() - expReplyLen = 0xFF indicates variable length expected
******************************************************************************/
BERR_Code SATFE_Ftm_SendMessage(SATFE_Chip *pChip, uint8_t *pSendBuf, uint8_t sendLen, uint8_t *pRcvBuf, int timeoutMsec, uint8_t expReplyLen, uint8_t expReplyCmd)
{
   BERR_Code retCode = BERR_SUCCESS;
   bool bGotExpMessage = false;
   uint8_t len;

   if (sendLen > 0)
   {
      SATFE_MUTEX(retCode = BAST_WriteFtm(pChip->hAst, pSendBuf, sendLen));
      if (retCode == BERR_SUCCESS)
         SATFE_Ftm_LogWrite(pChip, pSendBuf, sendLen);
   }

   while ((expReplyLen > 0) && (retCode == BERR_SUCCESS) && !bGotExpMessage)
   {
      if ((retCode = SATFE_Ftm_GetMessage(pChip, pRcvBuf, &len, timeoutMsec)) != BERR_SUCCESS)
         break;

      if (((len == expReplyLen) || (expReplyLen == 0xFF)) && (pRcvBuf[0] == expReplyCmd))
         bGotExpMessage = true;
      else
         printf("%02x %02x ***** GOT UNEXPECTED MESSAGE ***** %02x %02x\n",len,expReplyLen,pRcvBuf[0],expReplyCmd);
   }

   return retCode;
}


/******************************************************************************
 SATFE_Ftm_GetMessage()
******************************************************************************/
BERR_Code SATFE_Ftm_GetMessage(SATFE_Chip *pChip, uint8_t *pBuf, uint8_t *pLen, int timeoutMsec)
{
   BERR_Code retCode;
   bool bGotMessage = false;

   SATFE_Platform_StartTimer();
   while ((SATFE_Platform_GetTimerCount() < (uint32_t)timeoutMsec) && !bGotMessage)
   {
      SATFE_OnIdle();
      if (BKNI_WaitForEvent(pChip->hFtmMessageEvent, 0) == BERR_SUCCESS)
         bGotMessage = true;
   }
   SATFE_Platform_KillTimer();

   if (!bGotMessage)
      retCode = BERR_TIMEOUT;
   else if ((retCode = BKNI_AcquireMutex(pChip->hFtmMessageMutex)) == BERR_SUCCESS)
   {
      *pLen = pChip->ftmMessageBuf[0] & 0x7F;
      if (*pLen)
         BKNI_Memcpy(pBuf, &(pChip->ftmMessageBuf[1]), *pLen);
      BKNI_ReleaseMutex(pChip->hFtmMessageMutex);
   }
   else
      printf("unable to acquire hFtmMessageMutex\n");

   return retCode;
}


/******************************************************************************
 SATFE_Ftm_Register()
******************************************************************************/
BERR_Code SATFE_Ftm_Register(SATFE_Chip *pChip, uint8_t num_tuners)
{
   BERR_Code retCode;
   uint8_t i, buf[16];

   /* reset the ftm uC */
   retCode = SATFE_Ftm_ResetUc(pChip);
   if (retCode != BERR_SUCCESS)
   {
      printf("SATFE_Ftm_ResetUc() error 0x%02X\n", retCode);
      return retCode;
   }

   /* register the tuners */
   for (i = 0; i < num_tuners; i++)
   {
      printf("Registering tuner%d to the SWM (RID=0x%08X)...\n", i, pChip->ftmRid);
      buf[0] = 0x0B;
      buf[1] = 0x01;
      buf[2] = 0x0F;
      buf[3] = 0x00;
      buf[4] = 0x06;
      buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);
      buf[6] = (uint8_t)((pChip->ftmRid >> 24) & 0xFF);
      buf[7] = (uint8_t)((pChip->ftmRid >> 16) & 0xFF);
      buf[8] = (uint8_t)((pChip->ftmRid >> 8) & 0xFF);
      buf[9] = (uint8_t)(pChip->ftmRid & 0xFF);
      buf[10] = 0x02 + i;
      buf[11] = SATFE_GetStreamCrc8(&buf[6], 5);

      if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, 12, buf, 4000, 14, 0x41)) == BERR_SUCCESS)
      {
         pChip->ftmTunerRegAddr[i] = buf[5] & 0x0F;
         printf("   --> tuner%d is assigned address 0x%02X\n", i, pChip->ftmTunerRegAddr[i]);
      }
      else
      {
         printf("tuner%d registration failed! <abort>\n", i);
         goto done;
      }

      BKNI_Sleep(2000);
   }

   done:
   return retCode;
}


/******************************************************************************
 SATFE_Command_ftm_uc_reset()
******************************************************************************/
bool SATFE_Command_ftm_uc_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_uc_reset", "ftm_uc_reset", "Initialize the FTM/FSK block.", "none", true);
      return true;
   }

   if (SATFE_Ftm_ResetUc(pChip) == BERR_SUCCESS)
      return true;

   return false;
}


/******************************************************************************
 SATFE_Command_ftm_write()
******************************************************************************/
bool SATFE_Command_ftm_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[128], i, n;

   if (argc < 2)
   {
      SATFE_PrintDescription1("ftm_write", "ftm_write [hex8 ...]", "Sends a message to uC or a packet to SWM network.", "data comprising entire encapsulated message (includes the initial cmd/length byte", true);
      return true;
   }

   for (i = 1; i < argc; i++)
      buf[i-1] = (uint8_t)strtoul(argv[i], NULL, 16);
   n = argc - 1;

   SATFE_MUTEX(retCode = BAST_WriteFtm(pChip->hAst, buf, n));
   if (retCode)
   {
      printf("BAST_WriteFtm() error 0x%X\n", retCode);
      return false;
   }

   SATFE_Ftm_LogWrite(pChip, buf, n);
   return true;
}

/******************************************************************************
 SATFE_Command_ftm_read()
******************************************************************************/
bool SATFE_Command_ftm_read(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[128], n;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_read", "ftm_read", "Reads a message from the uC.", "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BAST_ReadFtm(pChip->hAst, buf, &n));
   if (retCode)
   {
      printf("BAST_ReadFtm() error 0x%X\n", retCode);
      return false;
   }

   SATFE_Ftm_LogRead(pChip, buf, n);
   return true;
}


/******************************************************************************
 SATFE_Command_ftm_uc_version()
******************************************************************************/
bool SATFE_Command_ftm_uc_version(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[16];

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_uc_version", "ftm_uc_version", "Send GET_VERSION local message to the uC.", "none", true);
      return true;
   }

   buf[0] = 0x81;
   buf[1] = 0x06;
   if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, 2, buf, 2000, 10, 0x06)) == BERR_SUCCESS)
   {
      printf("firmware image index = 0x%02X\n", buf[1]);
      printf("major version = 0x%02X\n", buf[2]);
      printf("minor version = 0x%02X\n", buf[3]);
      printf("build number  = 0x%02X%02X\n", buf[4], buf[5]);
   }

   SATFE_RETURN_ERROR("SATFE_Command_ftm_uc_version()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_options()
******************************************************************************/
bool SATFE_Command_ftm_options(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint16_t options;
   uint8_t len, exp_len, exp_cmd, buf[16];

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("ftm_options", "ftm_options <options>",
                              "Send GET_OPTIONS or SET_OPTIONS local command.",
                              "options = (optional) 16-bit hex", true);
      return true;
   }

   if (argc == 1)
   {
      /* get_options */
      buf[0] = 0x81;
      buf[1] = 0x05;
      len = 2;
      exp_len = 3;
      exp_cmd = 0x05;
   }
   else
   {
      /* set_options */
      options = (uint16_t)strtoul(argv[1], NULL, 16);
      buf[0] = 0x83;
      buf[1] = 0x04;
      buf[2] = (options >> 8) & 0xFF;
      buf[3] = options & 0xFF;
      len = 4;
      exp_len = 0;
      exp_cmd = 0x00;
   }

   if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, len, buf, 2000, exp_len, exp_cmd)) == BERR_SUCCESS)
   {
      if (exp_cmd == 0x05)
         printf("Options = 0x%02X%02X\n", buf[1], buf[2]);
   }

   SATFE_RETURN_ERROR("SATFE_Command_ftm_options()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_rx_mask()
******************************************************************************/
bool SATFE_Command_ftm_rxmask(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint16_t mask;
   uint8_t len, exp_len, exp_cmd, buf[16];

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("ftm_rxmask", "ftm_rxmask <mask>",
                              "Send GET_RX_BIT_MASK or SET_RX_BIT_MASK local command.",
                              "mask = (optional) 16-bit hex", true);
      return true;
   }

   if (argc == 1)
   {
      /* get_rx_bit_mask */
      buf[0] = 0x81;
      buf[1] = 0x03;
      len = 2;
      exp_len = 3;
      exp_cmd = 0x03;
   }
   else
   {
      /* set_rx_bit_mask */
      mask = (uint16_t)strtoul(argv[1], NULL, 16);
      buf[0] = 0x83;
      buf[1] = 0x02;
      buf[2] = (mask >> 8) & 0xFF;
      buf[3] = mask & 0xFF;
      len = 4;
      exp_len = 0;
      exp_cmd = 0x00;
   }

   if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, len, buf, 2000, exp_len, exp_cmd)) == BERR_SUCCESS)
   {
      if (exp_cmd == 0x03)
         printf("Rx Mask = 0x%02X%02X\n", buf[1], buf[2]);
   }

   SATFE_RETURN_ERROR("SATFE_Command_ftm_rxmask()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_hard_reset()
******************************************************************************/
bool SATFE_Command_ftm_hard_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[2];

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_hard_reset", "ftm_hard_reset",
                              "Send a FTM hard reset command.", "none", true);
      return true;
   }

   buf[0] = 0x81;
   buf[1] = 0x08;

   retCode = BAST_WriteFtm(pChip->hAst, buf, 2);
   if (retCode == BERR_SUCCESS)
      SATFE_Ftm_LogWrite(pChip, buf, 2);

   SATFE_RETURN_ERROR("SATFE_Command_ftm_hard_reset()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_register()
******************************************************************************/
bool SATFE_Command_ftm_register(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[16], tuner_index;

   if (argc != 2)
   {
      SATFE_PrintDescription1("ftm_register", "ftm_register [tuner_index_hex]",
                              "Send a registration request packet.",
                              "tuner_index_hex = 0x00 to 0x0F", false);
      SATFE_PrintDescription2("4-bit tuner_index field in payload of the registration request packet", true);
      return true;
   }

   tuner_index = (uint8_t)strtoul(argv[1], NULL, 16);
   if (tuner_index > 15)
   {
      printf("tuner_index out of range\n");
      return false;
   }

   printf("FTM RID = 0x%08X\n", pChip->ftmRid);
   buf[0] = 0x0B;
   buf[1] = 0x01;
   buf[2] = 0x0F;
   buf[3] = 0x00;
   buf[4] = 0x06;
   buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);
   buf[6] = (uint8_t)((pChip->ftmRid >> 24) & 0xFF);
   buf[7] = (uint8_t)((pChip->ftmRid >> 16) & 0xFF);
   buf[8] = (uint8_t)((pChip->ftmRid >> 8) & 0xFF);
   buf[9] = (uint8_t)(pChip->ftmRid & 0xFF);
   buf[10] = tuner_index;
   buf[11] = SATFE_GetStreamCrc8(&buf[6], 5);

   if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, 12, buf, 4000, 14, 0x41)) == BERR_SUCCESS)
      printf("   --> assigned src_addr is 0x%02X\n", buf[5]);

   SATFE_RETURN_ERROR("SATFE_Command_ftm_register()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_echo()
******************************************************************************/
bool SATFE_Command_ftm_echo(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[65], rcv_buf[65], i, n, src_addr, payload_length, errors;

   if (argc < 2)
   {
      SATFE_PrintDescription1("ftm_echo", "ftm_echo [src_addr_hex] <data_hex ...>",
                              "Send a network echo packet.",
                              "src_addr_hex = src_addr in packet header", false);
      SATFE_PrintDescription2("data_hex = 8-bit hexadecimal payload data", true);
      return true;
   }

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 15)
   {
      printf("src_addr out of range\n");
      return false;
   }

   n = argc - 2;
   if (n > 58)
   {
      printf("too many data bytes to fit into the payload\n");
      return false;
   }

   payload_length = n ? (n+1) : 0;

   buf[0] = 5 + payload_length; /*6 + n;*/
   buf[1] = 0x17;
   buf[2] = src_addr;
   buf[3] = 0x00;
   buf[4] = payload_length; /*n + 1;*/
   buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);

   if (payload_length > 0)
   {
      for (i = 0; i < n; i++)
      {
         buf[6+i] = (uint8_t)strtoul(argv[2+i], NULL, 16);
      }
      buf[6+n] = SATFE_GetStreamCrc8(&buf[6], (uint8_t)n);
   }

   retCode = SATFE_Ftm_SendMessage(pChip, buf, (uint8_t)(payload_length + 6), rcv_buf, 2000, (uint8_t)(payload_length + 5), 0x57);
   if (retCode == BERR_SUCCESS)
   {
      errors = 0;
      for (i = 0; i < payload_length - 1; i++)
      {
         if (buf[6+i] != rcv_buf[5+i])
         {
            printf("received echo response is incorrect in byte %d (expected 0x%02X, got 0x%02X)\n", i, buf[6+i], rcv_buf[5+i]);
            errors++;
         }
      }
      if (errors == 0)
         printf("   --> correct echo response received\n");
   }
   else
      printf("failed to get echo response!\n");

   SATFE_RETURN_ERROR("SATFE_Command_ftm_echo()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_ping()
******************************************************************************/
bool SATFE_Command_ftm_ping(SATFE_Chip *pChip, int argc, char **argv)
{
   static uint8_t x = 0;
   BERR_Code retCode;
   uint8_t buf[65], rcv_buf[65], i, src_addr, dest_addr, payload_length,errors;

   if (argc != 4)
   {
      SATFE_PrintDescription1("ftm_ping", "ftm_ping [src_addr_hex] [dest_addr_hex] [payload_length]",
                              "Send a network ping packet to another IRD.",
                              "src_addr_hex = src_addr in packet header", false);
      SATFE_PrintDescription2("dest_addr_hex = dest_addr in packet header", false);
      SATFE_PrintDescription2("payload_length = payload length of ping packet in dec", true);
      return true;
   }

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 0x0F)
   {
      printf("src_addr out of range\n");
      return false;
   }

   dest_addr = (uint8_t)strtoul(argv[2], NULL, 16);
   if (src_addr > 0x0F)
   {
      printf("dest_addr out of range\n");
      return false;
   }

   payload_length = (uint8_t)atoi(argv[3]);
   if (payload_length > 58)
   {
      printf("payload_length out of range\n");
      return false;
   }

   buf[0] = 5 + payload_length;
   buf[1] = 0x19;
   buf[2] = src_addr;
   buf[3] = dest_addr;
   buf[4] = payload_length;
   buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);

   if (payload_length > 0)
   {
      for (i = 0; i < (payload_length - 1); i++)
         buf[6+i] = x++;
      buf[5+payload_length] = SATFE_GetStreamCrc8(&buf[6], (uint8_t)(payload_length - 1));
   }

   retCode = SATFE_Ftm_SendMessage(pChip, buf, (uint8_t)(payload_length + 6), rcv_buf, 3000, (uint8_t)(payload_length + 5), 0x59);
   if (retCode == BERR_SUCCESS)
   {
      errors = 0;
      for (i = 0; i < payload_length - 1; i++)
      {
         if (buf[6+i] != rcv_buf[5+i])
         {
            printf("received ping response is incorrect in byte %d (expected 0x%02X, got 0x%02X)\n", i, buf[6+i], rcv_buf[5+i]);
            errors++;
         }
      }
      if (errors == 0)
         printf("   --> correct ping response received\n");
   }
   else
      printf("ping failed!\n");

   SATFE_RETURN_ERROR("SATFE_Command_ftm_ping()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_xtune()
******************************************************************************/
bool SATFE_Command_ftm_xtune(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[16], i, src_addr, input = 1;
   uint32_t xtune_freq = 0x4060; /* 0x4060 * 62.5kHz = 1030MHz*/
   bool bFreq, bInput;

   if (argc < 2)
   {
      SATFE_PrintDescription1("ftm_xtune", "ftm_xtune [src_addr_hex] <-f freq_mhz> <-i input>", "Send extended tuning request.",
                              "src_addr_hex = src_addr in packet header", false);
      SATFE_PrintDescription2("freq = (optional, default=1030MHz) requested RF frequency", false);
      SATFE_PrintDescription2("input = (optional, default=1) select SWM input", true);
      return true;
   }

   /* init flags */
   bFreq = bInput = false;

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 15)
   {
      printf("src_addr out of range\n");
      return false;
   }

   /* parse optional arguments */
   for (i = 2; i < argc; i++)
   {
      if (bFreq)
      {
         bFreq = false;
         if (SATFE_GetFreqFromString(pChip, argv[i], &xtune_freq) == false)
         {
            printf("invalid frequency\n");
            return false;
         }
         xtune_freq = (uint32_t)(xtune_freq / 62500);
      }
      else if (bInput)
      {
         bInput = false;
         input = (uint8_t)atoi(argv[i]);
      }
      else if (!strncmp(argv[i], "-f", 2))
         bFreq = true;
      else if (!strncmp(argv[i], "-i", 2))
         bInput = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   /* format xtune message */
   buf[0] = 0x09;
   buf[1] = 0x05;
   buf[2] = src_addr;
   buf[3] = 0x00;
   buf[4] = 0x04;
   buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);
   buf[6] = input;   /* [7:6] reserved, [5] vtop, [4] tone on, [3:0] SWM input port select */
   buf[7] = (uint8_t)((xtune_freq >> 8) & 0xFF);
   buf[8] = (uint8_t)(xtune_freq & 0xFF);
   buf[9] = SATFE_GetStreamCrc8(&buf[6], 3);

   retCode = SATFE_Ftm_SendMessage(pChip, buf, 10, buf, 2000, 0xFF, 0x45);
   if (retCode == BERR_SUCCESS)
      printf("   --> received xtune response\n");
   else
      printf("failed to get xtune response!\n");

   SATFE_RETURN_ERROR("SATFE_Command_ftm_xtune()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_lock()
******************************************************************************/
bool SATFE_Command_ftm_lock(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[16], src_addr;

   if (argc != 2)
   {
      SATFE_PrintDescription1("ftm_lock", "ftm_lock [src_addr_hex]",
                              "Request ftm lock.",
                              "src_addr_hex = src_addr in packet header", true);
      return true;
   }

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 15)
   {
      printf("src_addr out of range\n");
      return false;
   }

   buf[0] = 5;
   buf[1] = 0x0E;
   buf[2] = src_addr;
   buf[3] = 0x00;
   buf[4] = 0x00;
   buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);

   retCode = SATFE_Ftm_SendMessage(pChip, buf, 6, buf, 2000, 5, 0x4E);
   if (retCode == BERR_SUCCESS)
      printf("   --> received lock response\n");
   else
      printf("failed to get lock response!\n");

   SATFE_RETURN_ERROR("SATFE_Command_ftm_lock()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_unlock()
******************************************************************************/
bool SATFE_Command_ftm_unlock(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[16], src_addr;

   if (argc != 2)
   {
      SATFE_PrintDescription1("ftm_unlock", "ftm_unlock [src_addr_hex]",
                              "Request ftm unlock.",
                              "src_addr_hex = src_addr in packet header", true);
      return true;
   }

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 15)
   {
      printf("src_addr out of range\n");
      return false;
   }

   buf[0] = 5;
   buf[1] = 0x0F;
   buf[2] = src_addr;
   buf[3] = 0x00;
   buf[4] = 0x00;
   buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);

   retCode = SATFE_Ftm_SendMessage(pChip, buf, 6, buf, 2000, 5, 0x4F);
   if (retCode == BERR_SUCCESS)
      printf("   --> received unlock response\n");
   else
      printf("failed to get unlock response!\n");

   SATFE_RETURN_ERROR("SATFE_Command_ftm_unlock()", retCode);
}


/******************************************************************************
 SATFE_Command_ftm_test_register()
******************************************************************************/
bool SATFE_Command_ftm_test_register(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint32_t max_retries = 3;
   uint32_t rand_delay, retries;
   uint32_t attempts = 0, success = 0;
   uint8_t num_tuners;
   uint8_t i, buf[64];

   if (argc != 2)
   {
      SATFE_PrintDescription1("ftm_test_register", "ftm_test_register [num_tuners]",
                              "Test FTM registration continuously.",
                              "num_tuners = number of tuners to register", true);
      return true;
   }

   num_tuners = (uint8_t)atoi(argv[1]);
   if ((num_tuners < 1) || (num_tuners > 5))
   {
      printf("invalid parameter\n");
      return false;
   }

   while (1)
   {
      /* reset the ftm uC */
      retCode = SATFE_Ftm_ResetUc(pChip);
      if (retCode != BERR_SUCCESS)
      {
         printf("SATFE_Ftm_ResetUc() error 0x%02X\n", retCode);
         return false;
      }

      /* wait some time for FTM to deregister existing tuners */
      rand_delay = 1600 + (rand() % 30);
      SATFE_Platform_StartTimer();
      while (SATFE_Platform_GetTimerCount() < rand_delay)
      {
         SATFE_OnIdle();
         if (SATFE_Platform_GetChar(false) > 0)
            goto abort_test;
      }
      SATFE_Platform_KillTimer();

      for (i = 0; i < num_tuners; i++)
      {
         for (retries = 0; retries < max_retries; retries++)
         {
            if (SATFE_Platform_GetChar(false) > 0)
               goto abort_test;

            attempts++;
            printf("Registering tuner%d to the FTM (RID=0x%08X)...\n", i, pChip->ftmRid);
            buf[0] = 0x0B;
            buf[1] = 0x01;
            buf[2] = 0x0F;
            buf[3] = 0x00;
            buf[4] = 0x06;
            buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);
            buf[6] = (uint8_t)((pChip->ftmRid >> 24) & 0xFF);
            buf[7] = (uint8_t)((pChip->ftmRid >> 16) & 0xFF);
            buf[8] = (uint8_t)((pChip->ftmRid >> 8) & 0xFF);
            buf[9] = (uint8_t)(pChip->ftmRid & 0xFF);
            buf[10] = 0x02 + i;
            buf[11] = SATFE_GetStreamCrc8(&buf[6], 5);

            if (SATFE_Ftm_SendMessage(pChip, buf, 12, buf, 4000, 14, 0x41) == BERR_SUCCESS)
            {
               printf("   --> tuner%d is assigned address 0x%02X\n", i, buf[5] & 0x0F);
               success++;
               break;
            }
            else
            {
               printf("tuner%d registration failed!\n", i);
               if ((buf[0] & 0x30) != 0x30)
                  goto abort_test;
            }
         }

         printf("%d / %d\n", success, attempts);

         if (retries >= max_retries)
         {
            abort_test:
            printf("test aborted!\n");
            return true;
         }

         if ((i + 1) < num_tuners)
         {
            rand_delay = 1000 + (rand() % 30);
            SATFE_Platform_StartTimer();
            while (SATFE_Platform_GetTimerCount() < rand_delay)
            {
               SATFE_OnIdle();
               if (SATFE_Platform_GetChar(false) > 0)
                  goto abort_test;
            }
            SATFE_Platform_KillTimer();
         }
      }
   }
   return true;
}


/******************************************************************************
 SATFE_Command_ftm_test_xtune()
******************************************************************************/
bool SATFE_Command_ftm_test_xtune(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_AcqSettings *pAcqSettings;

   uint32_t tuner_freq, xtune_freq = 0x4060; /* 0x4060 * 62.5kHz = 1030MHz*/
   uint32_t success = 0, attempts = 0;
   bool bLocked;
   uint8_t buf[16];

   BSTD_UNUSED(argv);
   pAcqSettings = &(pChip->acqSettings[pChip->currAcqSettings]);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_test_xtune", "ftm_test_xtune",
                              "Test FTM extended tuning continuously.", "none", true);
      return false;
   }

   /* register single tuner */
   if (SATFE_Ftm_Register(pChip, 1) != BERR_SUCCESS)
      return false;

   printf("Press <Enter> to quit\n");
   while (SATFE_Platform_GetChar(false) <= 0)
   {
      /* format xtune message */
      buf[0] = 0x09;
      buf[1] = 0x05;
      buf[2] = pChip->ftmTunerRegAddr[0];
      buf[3] = 0x00;
      buf[4] = 0x04;
      buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);
      buf[6] = 0x02; /* ftm input 2, vbot, tone off */ /* [7:6] reserved, [5] vtop, [4] tone on, [3:0] SWM input port select */
      buf[7] = (uint8_t)((xtune_freq >> 8) & 0xFF);
      buf[8] = (uint8_t)(xtune_freq & 0xFF);
      buf[9] = SATFE_GetStreamCrc8(&buf[6], 3);
      attempts++;
      /* send xtune packet */
      if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, 10, buf, 2000, 0xFF, 0x45)) != BERR_SUCCESS)
      {
         printf("ERROR: did not receive the xtune response!\n");
         goto done;
      }

      /* tune acquire */
      tuner_freq = 974 + 102 * (pChip->ftmTunerRegAddr[0] - 1);
      printf(" Tuning Ch1 to allocated %d MHz... ", tuner_freq);
      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], tuner_freq * 1000000, pAcqSettings));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_TuneAcquire() error 0x%X\n", retCode);
         goto done;
      }

      BKNI_Sleep(500);

      /* get lock status */
      retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked);
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetLockStatus() error 0x%X\n", retCode);
         goto done;
      }
      if (bLocked)
      {
         printf("(LOCKED)\n");
         success++;
      }
      else
         printf("NOT LOCKED!\n");

      /* alternate frequencies */
      if (xtune_freq == 0x4060)
         xtune_freq = 0x4920; /* 1170 MHz */
      else
         xtune_freq = 0x4060; /* 1030 MHz */
   }

   if (attempts)
   {
      printf("\n");
      printf("xtune requests sent  = %d\n", attempts);
      printf("successful locks = %d\n", success);
   }

   done:
   return true;
}

#define FTM_MAX_TUNERS 12
/******************************************************************************
 bool SATFE_Command_ftm_test_echo()
******************************************************************************/
bool SATFE_Command_ftm_test_echo(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t max_attempts, success[FTM_MAX_TUNERS], attempts[FTM_MAX_TUNERS];
   bool bQuitOnError, bIterations, bNumTuners, bPacketLength, bRandomLength, bPacketContent;
   bool bRespRcvd, bRespValid, bTimerExpired;

   uint8_t buf[64], rxbuf[64], rand[58], pktlen, errors;
   uint8_t i, currTuner, pktDropped, pktBad, errAst, errTotal;
   uint8_t num_ftm_tuners, payload_len, payload_content;
   uint8_t modePacket;  /* 0=(default) sequential, 1=user specified, 2=random */

   if ((argc < 1) || (argc > 10))
   {
      SATFE_PrintDescription1("ftm_test_echo",
                              "ftm_test_echo <-u> <-c max_attempts> <-t num_ftm_tuners> <-p payload_len> <-d payload_content>",
                              "Test FTM echo continuously.", "-u = (optional, default=disabled) abort test on first error", false);
      SATFE_PrintDescription2("max_attempts = (optional, default=inf) number of echo packets per tuner", false);
      SATFE_PrintDescription2("num_ftm_tuners = (optional, default=2) number of tuners to register", false);
      SATFE_PrintDescription2("payload_len = (optional, default=10) payload length of echo packet", false);
      SATFE_PrintDescription2("              r for random length", false);
      SATFE_PrintDescription2("payload_content = (optional, default=sequential) byte content of payload", false);
      SATFE_PrintDescription2("              r for random content", true);
      return false;
   }

   /* init flags and counters */
   bQuitOnError = bIterations = bNumTuners = bPacketLength = bRandomLength = bPacketContent = false;
   currTuner = pktDropped = pktBad = errAst = errTotal = 0;
   bRespRcvd = bRespValid = bTimerExpired = true;

   for (i = 0; i < FTM_MAX_TUNERS; i++)
   {
      success[i] = 0;
      attempts[i] = 0;
   }

   /* default values for optional arguments */
   max_attempts = 0;
   num_ftm_tuners = 2;
   payload_len = 10;
   payload_content = 0;
   modePacket = 0;

   /* parse optional arguments */
   for (i = 1; i < argc; i++)
   {
      if (bIterations)
      {
         bIterations = false;
         max_attempts = (uint32_t)atoi(argv[i]);
      }
      else if (bNumTuners)
      {
         bNumTuners = false;
         num_ftm_tuners = (uint8_t)atoi(argv[i]);

         if ((num_ftm_tuners < 1) || (num_ftm_tuners > FTM_MAX_TUNERS))
         {
            printf("invalid num_ftm_tuners\n");
            return false;
         }
      }
      else if (bPacketLength)
      {
         bPacketLength = false;

         /* check for random packet length option */
         if (!strncmp(argv[i], "r", 1))
            bRandomLength = true;
         else
            payload_len = (uint8_t)atoi(argv[i]);

         /* cap payload length */
         if (payload_len > 58)
            payload_len = 58;
      }
      else if (bPacketContent)
      {
         bPacketContent = false;

         /* check for random packet content option */
         if (!strncmp(argv[i], "r", 1))
            modePacket = 2;
         else
         {
            /* user-specified payload */
            modePacket = 1;
            payload_content = (uint8_t)strtoul(argv[i], NULL, 16);
         }
      }
      else if (!strncmp(argv[i], "-u", 2))
         bQuitOnError = true;
      else if (!strncmp(argv[i], "-c", 2))
         bIterations = true;
      else if (!strncmp(argv[i], "-t", 2))
         bNumTuners = true;
      else if (!strncmp(argv[i], "-p", 2))
         bPacketLength = true;
      else if (!strncmp(argv[i], "-d", 2))
         bPacketContent = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   /* register single tuner */
   if (SATFE_Ftm_Register(pChip, num_ftm_tuners) != BERR_SUCCESS)
   {
      printf("failed to register tuners\n");
      return false;
   }

   printf("Press <Enter> to quit\n");
   while (SATFE_Platform_GetChar(false) <= 0)
   {
      if (bRespValid || bRespRcvd || bTimerExpired)
      {
         if (bTimerExpired && !bRespRcvd)
         {
            printf("ERROR: did not receive the echo response\n");
            pktDropped++;
            if (bQuitOnError)
               goto done;
         }
         else if (bRespRcvd && !bRespValid)
         {
            printf("ERROR: bad echo response received\n");
            pktBad++;
            if (bQuitOnError)
               goto done;
         }

         if (max_attempts && (attempts[0] >= max_attempts))
            goto done;

         BKNI_Sleep(attempts[currTuner] & 500);

         bRespRcvd = bRespValid = bTimerExpired = false;
         if (attempts[currTuner] > 0)
            printf("tuner%d: %d/%d\n", currTuner, success[currTuner], attempts[currTuner]);

         if (bRandomLength)
            payload_len = (uint8_t)(SATFE_Platform_Rand() % 58);

         /* send the echo packet */
         currTuner++;
         if (currTuner >= num_ftm_tuners)
            currTuner = 0;
         buf[0] = 6 + payload_len;  /* packet length */
         buf[1] = 0x17;             /* echo command */
         buf[2] = pChip->ftmTunerRegAddr[currTuner];
         buf[3] = 0x00;             /* dest address */
         buf[4] = payload_len ? payload_len + 1 : 0x00;
         buf[5] = SATFE_GetStreamCrc8(&buf[1], 4);
         for (i = 0; i < payload_len; i++)
         {
            if (modePacket == 2)
            {
               buf[6+i] = (uint8_t)SATFE_Platform_Rand();
               rand[i] = buf[6+i];   /* store data for validation */
            }
            else if (modePacket == 1)
               buf[6+i] = payload_content;
            else
               buf[6+i] = (uint8_t)(i + attempts[currTuner]);
         }
         if (payload_len > 0)
            buf[6+i] = SATFE_GetStreamCrc8(&buf[6], payload_len);

         printf("tuner%d sending echo packet %d...\n", currTuner, attempts[currTuner]++);

         retCode = BAST_WriteFtm(pChip->hAst, buf, (uint8_t)(payload_len + 7));
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_WriteFtm() error 0x%X\n", retCode);
            errAst++;
            if (bQuitOnError)
               goto done;
         }
         SATFE_Platform_StartTimer();
      }

      /* wait for echo response */
      SATFE_OnIdle();
      if ((retCode = BKNI_WaitForEvent(pChip->hFtmMessageEvent, 0)) == BERR_SUCCESS)
      {
         if (BKNI_AcquireMutex(pChip->hFtmMessageMutex) == BERR_SUCCESS)
         {
            pktlen = pChip->ftmMessageBuf[0] & 0x7F;
            BKNI_Memcpy(rxbuf, &(pChip->ftmMessageBuf[1]), pktlen);
            BKNI_ReleaseMutex(pChip->hFtmMessageMutex);

            /* validate header */
            if (pktlen == (payload_len ? payload_len + 6 : 5))
            {
               /* check command byte and dest address */
               if ((rxbuf[0] == 0x57) && ((rxbuf[2] & 0x0F) == pChip->ftmTunerRegAddr[currTuner]))
                  bRespRcvd = true; /* this is an echo response packet */
            }

            if (bRespRcvd)
            {
               /* validate echo response crc */
               if (rxbuf[5+payload_len] != SATFE_GetStreamCrc8(&rxbuf[5], payload_len))
               {
                  printf("Payload CRC error! (expected 0x%02X, got 0x%02X)\n",
                        SATFE_GetStreamCrc8(&rxbuf[5], payload_len),
                        rxbuf[5+payload_len]);
                  errors = 1;
                  goto payload_error;
               }

               /* validate echo response data */
               errors = 0;
               for (i = 0; i < payload_len; i++)
               {
                  if (modePacket == 2)
                  {
                     /* validate random data */
                     if (rxbuf[5+i] != rand[i])
                        errors++;
                  }
                  else if (modePacket == 1)
                  {
                     /* validate user data */
                     if (rxbuf[5+i] != payload_content)
                        errors++;
                  }
                  else
                  {
                     /* validate sequential data */
                     if ((rxbuf[5+i] + 1) != (uint8_t)(attempts[currTuner] + i))
                        errors++;
                  }
               }

               if (errors == 0)
               {
                  bRespValid = true;
                  success[currTuner]++;
               }
               else
               {
                  payload_error:
                  errTotal = errTotal + errors;
                  if (bQuitOnError)
                     goto done;
               }
            }
         }
         else
         {
            printf("failed to acquire hFtmMessageMutex\n");
            if (bQuitOnError)
               goto done;
         }
      }
      else if (retCode != BERR_TIMEOUT)
         printf("BKNI_WaitForEvent() error 0x%X\n", retCode);

      if (!bRespValid && (SATFE_Platform_GetTimerCount() > 5000))
         bTimerExpired = true;
   }

   done:
   SATFE_Platform_KillTimer();

   if (attempts[0])
   {
      printf("\n");
      for (i = 0; i < num_ftm_tuners; i++)
         printf("TUNER%d: %d echo packets sent, %d correct echo responses received\n", i, attempts[i], success[i]);
      printf("\nDROPPED PACKETS: %d\n", pktDropped);
      printf("BAD PACKETS: %d\n", pktBad);
      printf("AST ERRORS: %d\n", errAst);
      printf("TOTAL ERRORS: %d\n", errTotal);
   }

   SATFE_RETURN_ERROR("SATFE_Command_ftm_test_echo()", retCode);
}


/******************************************************************************
 bool SATFE_Command_ftm_power_down()
******************************************************************************/
bool SATFE_Command_ftm_power_down(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_power_down", "ftm_power_down", "Power down FTM core.", "none", true);
      return true;
   }

   retCode = BAST_PowerDownFtm(pChip->hAst);
   SATFE_RETURN_ERROR("SATFE_Command_ftm_power_down()", retCode);
}


/******************************************************************************
 bool SATFE_Command_ftm_power_up()
******************************************************************************/
bool SATFE_Command_ftm_power_up(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_power_up", "ftm_power_up", "Power up FTM core.", "none", true);
      return true;
   }

   retCode = BAST_PowerUpFtm(pChip->hAst);
   SATFE_RETURN_ERROR("SATFE_Command_ftm_power_up()", retCode);
}
#endif


/******************************************************************************
 SATFE_Command_bcm3445_config()
******************************************************************************/
bool SATFE_Command_bcm3445_config(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_Bcm3445Settings settings;
   uint8_t mi2c, out1, out2, daisy;

   if (argc != 5)
   {
      SATFE_PrintDescription1("bcm3445_config", "bcm3445_config [mi2c] [out1] [out2] [daisy]",
                              "Configure the external BCM3445.", "mi2c = which mi2c channel controls the BCM3445 (0 or 1)", false);
      SATFE_PrintDescription2("out1, out2, daisy = one of the following:", false);
      SATFE_PrintDescription2("   0 : powered down", false);
      SATFE_PrintDescription2("   1 : DAISY", false);
      SATFE_PrintDescription2("   2 : IN1(VGA)", false);
      SATFE_PrintDescription2("   3 : IN1(DB)", false);
      SATFE_PrintDescription2("   4 : IN2(VGA)", true);
      return true;
   }

   mi2c = (uint8_t)atoi(argv[1]);
   out1 = (uint8_t)atoi(argv[2]);
   out2 = (uint8_t)atoi(argv[3]);
   daisy = (uint8_t)atoi(argv[4]);
   if ((out1 > 4) || (out2 > 4) || (daisy > 4) || (daisy == 1) || (mi2c > 1))
   {
      printf("invalid parameter\n");
      return false;
   }
   settings.mi2c = mi2c;
   settings.out1 = out1;
   settings.out2 = out2;
   settings.daisy = daisy;
   SATFE_MUTEX(retCode = BAST_ConfigBcm3445(pChip->hAst, &settings));
   SATFE_RETURN_ERROR("SATFE_Command_bcm3445_config()", retCode);
}


/******************************************************************************
 SATFE_Command_bcm3445_map_output()
******************************************************************************/
bool SATFE_Command_bcm3445_map_output(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t mi2c, out;

   if (argc != 3)
   {
      SATFE_PrintDescription1("bcm3445_map_output", "bcm3445_map_output [mi2c] [out]",
                              "Map bcm3445 output to this channel's tuner input.", "mi2c = which mi2c channel controls the BCM3445 (0 or 1)", false);
      SATFE_PrintDescription2("out = one of the following:", false);
      SATFE_PrintDescription2("   0 : none", false);
      SATFE_PrintDescription2("   1 : OUT1", false);
      SATFE_PrintDescription2("   2 : OUT2", false);
      SATFE_PrintDescription2("   3 : DAISY", true);
      return true;
   }

   mi2c = (uint8_t)atoi(argv[1]);
   out = (uint8_t)atoi(argv[2]);
   if ((mi2c > 1) || (out > 3))
   {
      printf("invalid parameter\n");
      return false;
   }

   SATFE_MUTEX(retCode = BAST_MapBcm3445ToTuner(pChip->hAstChannel[pChip->currChannel],
                                                (BAST_Mi2cChannel)mi2c, (BAST_Bcm3445OutputChannel)out));
   SATFE_RETURN_ERROR("SATFE_Command_bcm3445_map_output()", retCode);
}


/******************************************************************************
 SATFE_Command_bcm3445_status()
******************************************************************************/
bool SATFE_Command_bcm3445_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_Bcm3445Status status;
   static char *bcm3445_output_port_str[] =
   {
      "none",
      "OUT1",
      "OUT2",
      "DAISY"
   };
   static char *bcm3445_output_config_str[] =
   {
      "powered off",
      "DAISY",
      "IN1(VGA)",
      "IN1(DB)",
      "IN2(VGA)"
   };

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("bcm3445_status", "bcm3445_status",
                              "Get BCM3445 status.", "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BAST_GetBcm3445Status(pChip->hAstChannel[pChip->currChannel], &status));
   SATFE_GOTO_DONE_ON_ERROR("BAST_GetBcm3445Status()", retCode);

   printf("MI2C control channel = %d\n", status.mi2c);
   printf("BCM3445 output port  = %s\n", status.tuner_input <= 3 ? bcm3445_output_port_str[status.tuner_input] : "not mapped");
   printf("BCM3445 out config   = %s\n", status.out_cfg <= 4 ? bcm3445_output_config_str[status.out_cfg] : "unknown");
   printf("status               = 0x%02X\n", status.status);
   printf("version              = 0x%02X\n", status.version);
   printf("agc                  = 0x%02X\n", status.agc);

   done:
   SATFE_RETURN_ERROR("SATFE_Command_bcm3445_status()", retCode);
}


#define BCM3445_ADDRESS 0xD8
/******************************************************************************
 SATFE_Command_bcm3445_read()
******************************************************************************/
bool SATFE_Command_bcm3445_read(SATFE_Chip *pChip, int argc, char **argv)
{
   static uint8_t bcm3445_registers_to_dump[] =
   {
      0x00, 0x01, 0x02, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
      0x10, 0x11, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1E, 0xFF
   };
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t val8, addr;
   int i;

   if ((argc != 2) && (argc != 1))
   {
      SATFE_PrintDescription1("bcm3445_read", "bcm3445_read [register_name]",
                              "Read a BCM3445 register.", "register_name = BCM3445 register", true);
      return true;
   }

   if (argc == 1)
   {
      i = 0;
      while ((addr = bcm3445_registers_to_dump[i]) != 0xFF)
      {
         SATFE_MUTEX(retCode = BAST_ReadMi2c(pChip->hAstChannel[pChip->currChannel], BCM3445_ADDRESS, &addr, 1, &val8, 1));
         if (retCode == BERR_SUCCESS)
            printf("register 0x%X = 0x%02X (%d)\n", addr, val8, val8);
         i++;
      }
   }
   else if (!strncmp(argv[1], "0x", 2))
   {
      addr = (uint8_t)strtoul(argv[1], NULL, 16);
      SATFE_MUTEX(retCode = BAST_ReadMi2c(pChip->hAstChannel[pChip->currChannel], BCM3445_ADDRESS, &addr, 1, &val8, 1));
      if (retCode == BERR_SUCCESS)
         printf("%s = 0x%02X (%d)\n", argv[1], val8, val8);
   }
   else
   {
      printf("unknown register\n");
      return false;
   }

   SATFE_RETURN_ERROR("SATFE_Command_bcm3445_read()", retCode);
}

/******************************************************************************
 SATFE_Command_bcm3445_write()
******************************************************************************/
bool SATFE_Command_bcm3445_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t val, buf[2];

   if (argc != 3)
   {
      SATFE_PrintDescription1("bcm3445_write", "bcm3445_write [register_name] [value]",
                              "Write to a BCM3445 register.", "register_name = BCM3445 register", false);
      SATFE_PrintDescription2("value = 8-bit value to write", true);
      return true;
   }

   if (!strncmp(argv[2], "0x", 2))
      val = (uint8_t)strtoul(argv[2], NULL, 16);
   else
      val = (uint8_t)atoi(argv[2]);

   if (!strncmp(argv[1], "0x", 2))
   {
      buf[0] = (uint8_t)strtoul(argv[1], NULL, 16);
      buf[1] = val;
      SATFE_MUTEX(retCode = BAST_WriteMi2c(pChip->hAstChannel[pChip->currChannel], BCM3445_ADDRESS, buf, 2));
   }
   else
   {
      printf("unknown register\n");
      return false;
   }

   SATFE_RETURN_ERROR("SATFE_Command_bcm3445_write()", retCode);
}


/******************************************************************************
 SATFE_Command_cwc()
******************************************************************************/
bool SATFE_Command_cwc(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   static BAST_SpurCancellerConfig spurConfig[6];
   static int nSpurs = 0;
   static bool bFirstTime = true;
   int i, j;
   uint32_t freq;

   if ((argc < 2) || (argc > 3))
   {
      SATFE_PrintDescription1("cwc",
                              "cwc list\n   cwc add [freq]\n   cwc delete [freq | all]",
                              "Manage spur cancellers.", "list = display list of spurs", false);
      SATFE_PrintDescription2("add = configures the list of spurs", false);
      SATFE_PrintDescription2("delete = Removes a single or all spur frequencies from the list.", false);
      return true;
   }

   if (bFirstTime)
   {
      /* initialize list of spurs */
      for (i = 0; i < 6; i++)
      {
         spurConfig[i].freq = 0;
      }
      nSpurs = 0;
      bFirstTime = false;
   }

   if (!strcmp(argv[1], "list"))
   {
      /* display the list */
      if (nSpurs == 0)
         printf("no spurs currently registered\n");
      else
      {
         for (i = 0; i < nSpurs; i++)
            printf("Spur %d: %u Hz\n", i, spurConfig[i].freq);
      }
   }
   else if (!strcmp(argv[1], "add") && (argc == 3))
   {
      if (nSpurs >= 6)
      {
         printf("no more spurs can be added\n");
         return true;
      }

      if (SATFE_GetFreqFromString(pChip, argv[2], &freq) == false)
         return false;

      /* ensure there are no duplicates in the list */
      for (i = 0; i < nSpurs; i++)
      {
         if (spurConfig[i].freq == freq)
            return true;
      }

      spurConfig[i].freq = freq;
      nSpurs++;

      commit:
      SATFE_MUTEX(retCode = BAST_EnableSpurCanceller(pChip->hAstChannel[pChip->currChannel], (uint8_t)nSpurs, spurConfig));
      SATFE_GOTO_DONE_ON_ERROR("BAST_EnableSpurCanceller", retCode);
   }
   else if (!strcmp(argv[1], "delete") && (argc == 3))
   {
      if (!strcmp(argv[2], "all"))
      {
         for (i = 0; i < 6; i++)
            spurConfig[i].freq = 0;
         nSpurs = 0;
      }
      else
      {
         if (SATFE_GetFreqFromString(pChip, argv[2], &freq) == false)
            return false;

         for (i = 0; i < nSpurs; i++)
         {
            if (spurConfig[i].freq == freq)
               break;
         }
         if (i >= nSpurs)
            printf("this frequency is not in the list\n");
         else
         {
            for (j = i; j < nSpurs; j++)
            {
               if (j < (nSpurs-1))
                  BKNI_Memcpy(&spurConfig[j], &spurConfig[j+1], sizeof(BAST_SpurCancellerConfig));
            }
            nSpurs--;
         }
      }
      goto commit;
   }
   else
   {
      printf("syntax error\n");
      retCode = BERR_INVALID_PARAMETER;
   }

   done:
   SATFE_RETURN_ERROR("SATFE_Command_cwc()", retCode);
}


/******************************************************************************
 SATFE_Command_delay()
******************************************************************************/
bool SATFE_Command_delay(SATFE_Chip *pChip, int argc, char **argv)
{
   uint32_t msecs;

   BSTD_UNUSED(pChip);

   if (argc != 2)
   {
      SATFE_PrintDescription1("delay",
                              "delay [msecs]",
                              "Sleep for a specified time", "msecs = number of milliseconds to sleep", true);
      return true;
   }

   msecs = (uint32_t)strtoul(argv[1], NULL, 10);

#if 1
   SATFE_Platform_StartTimer();
   while (SATFE_Platform_GetTimerCount() < msecs)
   {
      SATFE_OnIdle();
   }
   SATFE_Platform_KillTimer();
#else
   BKNI_Sleep(msecs);
#endif
   return true;
}


/******************************************************************************
 SATFE_Command_mi2c_write()
******************************************************************************/
bool SATFE_Command_mi2c_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int i;
   bool bChipAddrSpecified = false;
   uint8_t chip_addr = 0, channel, n = 0, buf[8];

   if (argc < 3)
   {
      SATFE_PrintDescription1("mi2c_write",
                              "mi2c_write <-c [channel]> [chip_hex_addr] [hex_data ...]",
                              "Initiate write transaction from MI2C controller", "channel = specifies the MI2C channel", false);
      SATFE_PrintDescription2("chip_hex_addr = slave chip I2C address in hexadecimal", false);
      SATFE_PrintDescription2("hex_data = 1 to 8 hexadecimal bytes to transmit", true);
      return true;
   }

   channel = pChip->currChannel;

   for (i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "-c"))
      {
         i++;
         if (i >= argc)
         {
            syntax_error:
            printf("syntax error\n");
            return false;
         }
         channel = atoi(argv[i]);
      }
      else if (!bChipAddrSpecified)
      {
         chip_addr = (uint8_t)strtoul(argv[i], NULL, 16);
         bChipAddrSpecified = true;
      }
      else
      {
         if (n >= 8)
            goto syntax_error;
         buf[n++] = (uint8_t)strtoul(argv[i], NULL, 16);
      }
   }

   if ((n == 0) || (n > 8) || !bChipAddrSpecified)
      goto syntax_error;

   if (pChip->chipFunctTable->Mi2cWrite == NULL)
   {
      if (channel >= pChip->chip.nDemods)
      {
         printf("invalid channel specified\n");
         return false;
      }
      SATFE_MUTEX(retCode = BAST_WriteMi2c(pChip->hAstChannel[channel], chip_addr, buf, n));
      SATFE_RETURN_ERROR("SATFE_Command_mi2c_write()", retCode);
   }
   else
      return pChip->chipFunctTable->Mi2cWrite(pChip, channel, chip_addr, buf, n);
}


/******************************************************************************
 SATFE_Command_mi2c_read()
******************************************************************************/
bool SATFE_Command_mi2c_read(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int i;
   bool bChipAddrSpecified = false;
   uint8_t chip_addr = 0, sub_addr, channel, n = 0, buf[8];

   if (argc < 4)
   {
      SATFE_PrintDescription1("mi2c_read",
                              "mi2c_read <-c [channel]> [chip_hex_addr] [sub_addr_hex] [n]",
                              "Initiate a read transaction from MI2C controller", "channel = specifies the MI2C channel", false);
      SATFE_PrintDescription2("chip_hex_addr = slave chip I2C address in hexadecimal", false);
      SATFE_PrintDescription2("sub_addr_hex = sub-address in hexadecimal", false);
      SATFE_PrintDescription2("n = number of bytes to receive (1 to 8)", true);
      return true;
   }

   channel = pChip->currChannel;

   for (i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "-c"))
      {
         i++;
         if (i >= argc)
         {
            syntax_error:
            printf("syntax error\n");
            return false;
         }
         channel = atoi(argv[i]);
      }
      else if (!bChipAddrSpecified)
      {
         if ((i + 3) > argc)
            goto syntax_error;
         chip_addr = (uint8_t)strtoul(argv[i++], NULL, 16);
         bChipAddrSpecified = true;
         sub_addr = (uint8_t)strtoul(argv[i++], NULL, 16);
         n = (uint8_t)strtoul(argv[i++], NULL, 10);
         if ((n < 1) || (n > 8))
         {
            printf("value out of range\n");
            return false;
         }
      }
      else
         goto syntax_error;
   }

   if (!bChipAddrSpecified)
      goto syntax_error;

   if (pChip->chipFunctTable->Mi2cRead == NULL)
   {
      if (channel >= pChip->chip.nDemods)
      {
         printf("invalid channel specified\n");
         return false;
      }
      SATFE_MUTEX(retCode = BAST_ReadMi2c(pChip->hAstChannel[channel], chip_addr, &sub_addr, 1, buf, n));
   }
   else
   {
      retCode = pChip->chipFunctTable->Mi2cRead(pChip, channel, chip_addr, &sub_addr, 1, buf, n);
   }

   if (retCode == BERR_SUCCESS)
   {
      printf("data read: ");
      for (i = 0; i < n; i++)
         printf("0x%02X ", buf[i]);
      printf("\n");
   }
   SATFE_RETURN_ERROR("SATFE_Command_mi2c_read()", retCode);
}


/******************************************************************************
 SATFE_Command_reset_channel()
******************************************************************************/
bool SATFE_Command_reset_channel(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bForce = false;

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("reset_channel <-f>",
                              "reset_channel",
                              "Soft reset current channel.", "-f = force reset even if other blocks (e.g. diseqc, mi2c) are busy", true);
      return true;
   }

   if (argc == 2)
   {
      if (!strcmp(argv[1], "-f"))
         bForce = true;
      else
      {
         printf("syntax error\n");
         return false;
      }
   }
   SATFE_MUTEX(retCode = BAST_ResetChannel(pChip->hAstChannel[pChip->currChannel], bForce));
   SATFE_RETURN_ERROR("BAST_ResetChannel()", retCode);
}


/******************************************************************************
 SATFE_Command_soft_reset()
******************************************************************************/
bool SATFE_Command_soft_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("soft_reset",
                              "soft_reset",
                              "Soft reset the frontend.", "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BAST_SoftReset(pChip->hAst));
   SATFE_RETURN_ERROR("BAST_SoftReset()", retCode);
}


/******************************************************************************
 SATFE_Command_bert()
******************************************************************************/
bool SATFE_Command_bert(SATFE_Chip *pChip, int argc, char **argv)
{
   uint32_t mask = 0;

   BDBG_ASSERT(pChip);

   if ((argc < 1) || (argc > 2))
   {
      SATFE_PrintDescription1("bert", "bert <on | off>", "Enable/disable bert acq_ctl", "on = enable bert", false);
      SATFE_PrintDescription2("off = disable bert", false);
      SATFE_PrintDescription2("(no option specified) = returns current setting", true);
      return true;
   }

   if (argc == 1)
   {
      if (pChip->acqSettings[pChip->currAcqSettings].acq_ctl & BAST_ACQSETTINGS_BERT_ENABLE)
         printf("bert is enabled in acquisition settings\n");
      else
         printf("bert is disabled in acquisition settings\n");
   }
   else
   {
      if (!strcmp(argv[1], "on"))
         mask = BAST_ACQSETTINGS_BERT_ENABLE | BAST_ACQSETTINGS_PN_INVERT | BAST_ACQSETTINGS_PN;
      else if (strcmp(argv[1], "off"))
      {
         printf("syntax error\n");
         return false;
      }

      pChip->acqSettings[pChip->currAcqSettings].acq_ctl &= ~(BAST_ACQSETTINGS_BERT_ENABLE | BAST_ACQSETTINGS_PN_INVERT | BAST_ACQSETTINGS_PN);
      pChip->acqSettings[pChip->currAcqSettings].acq_ctl |= mask;
   }

   return true;
}


/******************************************************************************
 SATFE_Command_pilot()
******************************************************************************/
bool SATFE_Command_pilot(SATFE_Chip *pChip, int argc, char **argv)
{
   uint32_t mask = 0;

   BDBG_ASSERT(pChip);

   if ((argc < 1) || (argc > 2))
   {
      SATFE_PrintDescription1("pilot", "pilot <on | off>", "Toggle pilot state in acq_ctl", "on = signal has pilot", false);
      SATFE_PrintDescription2("off = signal has no pilot", false);
      SATFE_PrintDescription2("(no option specified) = returns current pilot setting", true);
      return true;
   }

   if (argc == 1)
   {
      if (pChip->acqSettings[pChip->currAcqSettings].acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT)
         printf("pilot is enabled in acquisition settings\n");
      else
         printf("pilot is disabled in acquisition settings\n");
   }
   else
   {
      if (!strcmp(argv[1], "on"))
         mask = BAST_ACQSETTINGS_LDPC_PILOT;
      else if (strcmp(argv[1], "off"))
      {
         printf("syntax error\n");
         return false;
      }

      pChip->acqSettings[pChip->currAcqSettings].acq_ctl &= ~BAST_ACQSETTINGS_LDPC_PILOT;
      pChip->acqSettings[pChip->currAcqSettings].acq_ctl |= mask;
   }

   return true;
}


/******************************************************************************
 SATFE_Command_amc_scrambling_seq()
******************************************************************************/
bool SATFE_Command_amc_scrambling_seq(SATFE_Chip *pChip, int argc, char **argv)
{
   int idx;

   if (argc != 2)
   {
      SATFE_PrintDescription1("amc_scrambling_seq", "amc_scrambling_seq [idx]", "Set AMC scrambling sequence", "idx = scrambling sequence index", true);
      return true;
   }

   idx = atoi(argv[1]);
   if ((idx > 1016) || (idx < 0))
   {
      printf("invalid idx (%d)\n", idx);
      return false;
   }
   return SATFE_WriteAmcScramblingSeq(pChip, idx);
}


/******************************************************************************
 SATFE_Command_adc()
******************************************************************************/
bool SATFE_Command_adc(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int idx;
   uint8_t adcSelect, n;

   if ((argc != 1) && (argc != 2))
   {
      SATFE_PrintDescription1("adc", "adc <adc_idx>", "Set/Get ADC channel select", "adc_idx = ADC selection (0 to 3)", true);
      return true;
   }

   if (argc == 1)
   {
      retCode = BAST_GetAdcSelect(pChip->hAstChannel[pChip->currChannel], &adcSelect, &n);
	  if (retCode == BERR_NOT_SUPPORTED)
	  {
		 printf("command not supported\n");
		 return false;
	  }
	  else if (retCode != BERR_SUCCESS)
	  {
		 printf("BAST_GetAdcSelect() error 0x%X\n", retCode);
		 return false;
	  }

      printf("Num ADCs = %d\n", n);
      printf("ADC select = %d\n", adcSelect);
   }
   else
   {
      idx = atoi(argv[1]);
      if ((idx < 0) || (idx > 3))
         printf("invalid ADC select\n");
      else
      {
         adcSelect = (uint8_t)idx;
         retCode = BAST_SetAdcSelect(pChip->hAstChannel[pChip->currChannel], adcSelect);
         if (retCode == BERR_NOT_SUPPORTED)
            printf("command not supported\n");
         else if (retCode != BERR_SUCCESS)
            printf("BAST_SetAdcSelect() error 0x%X\n", retCode);
      }
   }

   return true;
}


/******************************************************************************
 SATFE_Command_diseqc_voltage()
******************************************************************************/
bool SATFE_Command_diseqc_voltage(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t v;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("diseqc_voltage", "diseqc_voltage", "Get DISEQC voltage raw value", "none", true);
      return true;
   }

   retCode = BAST_GetDiseqcVoltage(pChip->hAstChannel[pChip->currChannel], &v);
   if (retCode == BERR_SUCCESS)
      printf("value = %d\n", v);

   SATFE_RETURN_ERROR("BAST_GetDiseqcVoltage()", retCode);
}


/******************************************************************************
 SATFE_Command_diseqc_status()
******************************************************************************/
bool SATFE_Command_diseqc_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_DiseqcStatus status;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("diseqc_status", "diseqc_status", "Get DISEQC status", "none", true);
      return true;
   }

   retCode = BAST_GetDiseqcStatus(pChip->hAstChannel[pChip->currChannel], &status);
   if (retCode == BERR_SUCCESS)
   {
      printf("status          = %d\n", status.status);
      printf("reply expected  = %s\n", status.bReplyExpected ? "yes" : "no");
      printf("num reply bytes = %d\n", status.nReplyBytes);
   }

   SATFE_RETURN_ERROR("BAST_GetDiseqcVoltage()", retCode);
}


/******************************************************************************
 SATFE_Command_test_mpeg_count()
******************************************************************************/
bool SATFE_Command_test_mpeg_count(SATFE_Chip *pChip, int argc, char **argv)
{
   BAST_ChannelStatus status;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t t0, t1, tprev, cprev;

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("test_mpeg_count", "test_mpeg_count", "Test MPEG count status.  You need to lock the demod prior to running this command.", "none", true);
      return true;
   }

   SATFE_Platform_StartTimer();
   tprev = cprev = 0;
   while ((SATFE_Platform_GetChar(false) <= 0) && (retCode == BERR_SUCCESS))
   {
      t0 = SATFE_Platform_GetTimerCount();
      SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
      t1 = SATFE_Platform_GetTimerCount();
      if (retCode == BERR_SUCCESS)
      {
         printf("%d,%d,%d,%d,%u,%d\n", t0, t1, t1-t0, t0-tprev, status.mpegCount, status.mpegCount-cprev);
         tprev = t0;
         cprev = status.mpegCount;
         BKNI_Sleep(1000);
      }
   }

   SATFE_RETURN_ERROR("BAST_GetChannelStatus()", retCode);
}


/******************************************************************************
 SATFE_Command_test_lock_event()
******************************************************************************/
bool SATFE_Command_test_lock_event(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BKNI_EventHandle hLockEvent;
   uint32_t freq;
   bool bLocked;

   BSTD_UNUSED(argv);

   if (argc != 2)
   {
      SATFE_PrintDescription1("test_lock_event", "test_lock_event [freq]", "Test lock event.", "freq = tuning frequency", true);
      return true;
   }

   /* get tuning frequency -> freq */
   if (SATFE_GetFreqFromString(pChip, argv[1], &freq) == false)
      return false;

   BAST_GetLockStateChangeEventHandle(pChip->hAstChannel[pChip->currChannel], &hLockEvent);

   SATFE_MUTEX(BAST_AbortAcq(pChip->hAstChannel[pChip->currChannel]));
   BKNI_WaitForEvent(hLockEvent, 0);
   SATFE_Platform_StartTimer();
   SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], freq, &(pChip->acqSettings[pChip->currAcqSettings])));
   if (retCode)
   {
      printf("BAST_TuneAcquire() error 0x%X\n", retCode);
      return false;
   }

   while (SATFE_Platform_GetChar(false) <= 0)
   {
      retCode = BKNI_WaitForEvent(hLockEvent, 2000);
      if (retCode == BERR_SUCCESS)
      {
         retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked);
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_GetLockStatus() error 0x%X\n", retCode);
            return false;
         }

         printf("%u msecs: lock status = %d\n", SATFE_Platform_GetTimerCount(), bLocked);
      }
      else if (retCode == BERR_TIMEOUT)
      {
#if 0
         retCode = BAST_GetLockStatus(pChip->hAstChannel[pChip->currChannel], &bLocked);
         if (retCode != BERR_SUCCESS)
         {
            printf("BAST_GetLockStatus() error 0x%X\n", retCode);
            return false;
         }
         retCode = BKNI_WaitForEvent(hLockEvent, 0);
         if (retCode == BERR_SUCCESS)
            printf("lock change event was set but BKNI_WaitForEvent() timed out!\n");
         else
            printf("lock change event not recieved: lock status = %d\n", bLocked);
#endif
      }
      else
         printf("BKNI_WaitForEvent() error 0x%X\n", retCode);
   }

   SATFE_Platform_KillTimer();
   return true;
}


/******************************************************************************
 SATFE_Command_test_snr()
******************************************************************************/
bool SATFE_Command_test_snr(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_ChannelStatus status;
   uint32_t freq, i = 0;
   float min_snr, snr;
   bool bError = false;

   BSTD_UNUSED(argv);

   if (argc != 3)
   {
      SATFE_PrintDescription1("test_snr", "test_snr [freq] [min_snr]", "Test for minimum SNR after locking.", "freq = tuning frequency", false);
      SATFE_PrintDescription2("min_snr = minimum SNR for lock", true);
      return true;
   }

   if (SATFE_GetFreqFromString(pChip, argv[1], &freq) == false)
      return false;

   min_snr = (float)atof(argv[2]);

   while (!bError && (SATFE_Platform_GetChar(false) <= 0))
   {
      i++;
      printf("%d. Acquiring ", i);
      SATFE_MUTEX(retCode = BAST_TuneAcquire(pChip->hAstChannel[pChip->currChannel], freq, &(pChip->acqSettings[pChip->currAcqSettings])));

      while (SATFE_Platform_GetChar(false) <= 0)
      {
         SATFE_MUTEX(retCode = BAST_GetChannelStatus(pChip->hAstChannel[pChip->currChannel], &status));
         if (status.bDemodLocked)
         {
            snr = (float)(status.snrEstimate / 256.0);
            printf("--> Locked, SNR=%.3f", snr);
            if (snr < min_snr)
            {
               printf(" (SNR too low)");
               bError = true;
            }
            printf("\n");
            break;
         }
      }
   }

   SATFE_RETURN_ERROR("SATFE_Command_test_snr()", retCode);
}


/******************************************************************************
 SATFE_get_32bit_binary_string()
******************************************************************************/
char* SATFE_get_32bit_binary_string(uint32_t val32)
{
   static char binstring[41];
   int i, n = 0;

   for (i = 31; i >= 0; i--)
   {
      if (val32 & (1 << i))
         binstring[n] = '1';
      else
         binstring[n] = '0';
      n++;
      if (((i%4) == 0) && (i != 31) && (i != 0))
         binstring[n++] = '_';
   }
   binstring[n] = 0;
   return binstring;
}


void BKNI_AssertIsrContext_isr(const char *filename, unsigned lineno)
{
   BSTD_UNUSED(filename);
   BSTD_UNUSED(lineno);
   return;
}
