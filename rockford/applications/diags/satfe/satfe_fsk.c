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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "satfe.h"
#include "satfe_platform.h"
#include "satfe_fsk.h"
#include "bfsk.h"
#include "bfsk_4538.h"

#ifdef SATFE_USE_BFSK
#ifdef BFSK_PROTOCOL_ECHO

bool bFskInit = false;


/******************************************************************************
 SATFE_Fsk_Reset()
******************************************************************************/
BERR_Code SATFE_Fsk_Reset(SATFE_Chip *pChip)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[3] = {0x82, 0x07, 0x00};

   /* reset the fsk block */
   printf("\n*** Resetting FSK block ***\n");
   SATFE_MUTEX(retCode = BFSK_ResetChannel(pChip->hFskChannel[pChip->currFskChannel]));
   if (retCode != BERR_SUCCESS)
      printf("BFSK_ResetChannel() error 0x%02X\n", retCode);

   if (retCode == BERR_SUCCESS)
      bFskInit = true;

   return retCode;
}


/******************************************************************************
 SATFE_Ftm_LogWrite()
******************************************************************************/
void SATFE_Ftm_LogWrite(SATFE_Chip *pChip, uint8_t *buf, uint8_t len)
{
   uint8_t i;

   if (!(pChip->bEnableFtmLogging))
      return;

   if (len > 0)
   {
      printf("FSK tx: ");
      for (i = 0; i < len; i++)
      {
         if (i == 2)
            printf("(0x%02X) ", buf[i]);
         else
            printf("0x%02X ", buf[i]);
         if (i == 2)
            printf("| ");
      }
      printf("\n");
   }
}


/******************************************************************************
 SATFE_Ftm_LogRead()
******************************************************************************/
void SATFE_Ftm_LogRead(SATFE_Chip *pChip, uint8_t *buf, uint8_t len)
{
   uint8_t i;

   if (!(pChip->bEnableFtmLogging))
      return;

   if (len > 0)
   {
      printf("FSK rx [#%d]: ", pChip->ftmPacketCount, buf[0]);
      for (i = 0; i < len; i++)
      {
         if (i == 2)
            printf("(0x%02X) ", buf[i]);
         else
            printf("0x%02X ", buf[i]);
         if (i == 2)
            printf("| ");
      }
      printf("\n");
   }
}


/******************************************************************************
 SATFE_Fsk_SendMessage() - expReplyLen = 0xFF indicates variable length expected
******************************************************************************/
BERR_Code SATFE_Fsk_SendMessage(SATFE_Chip *pChip, uint8_t *pSendBuf, uint8_t timeSlot, uint8_t *pRcvBuf, int timeoutMsec, uint8_t expReplyCmd)
{
   BERR_Code retCode = BERR_SUCCESS;
   BFSK_P_TxParams txParams;
   bool bGotExpMessage = false;

   txParams.tdmaTxSlot = timeSlot;
   SATFE_MUTEX(retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], pSendBuf, 9, &txParams));
   if (retCode == BERR_SUCCESS)
      SATFE_Ftm_LogWrite(pChip, pSendBuf, 9);

   while ((retCode == BERR_SUCCESS) && !bGotExpMessage)
   {
      if ((retCode = SATFE_Fsk_GetMessage(pChip, pRcvBuf, timeoutMsec)) != BERR_SUCCESS)
         break;

      if (pRcvBuf[2] == expReplyCmd)
         bGotExpMessage = true;
      else
         printf(" ***** GOT UNEXPECTED MESSAGE ***** %02x %02x\n", pRcvBuf[2], expReplyCmd);
   }

   return retCode;
}


/******************************************************************************
 SATFE_Fsk_GetMessage()
******************************************************************************/
BERR_Code SATFE_Fsk_GetMessage(SATFE_Chip *pChip, uint8_t *pBuf, int timeoutMsec)
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
      BKNI_Memcpy(pBuf, &(pChip->ftmMessageBuf[0]), 9);
      BKNI_ReleaseMutex(pChip->hFtmMessageMutex);
   }
   else
      printf("unable to acquire hFtmMessageMutex\n");

   return retCode;
}


/******************************************************************************
 SATFE_Command_fsk_reset()
******************************************************************************/
bool SATFE_Command_fsk_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("fsk_reset", "fsk_reset", "Initialize the FSK block.", "none", true);
      return true;
   }

   retCode = SATFE_Fsk_Reset(pChip);
   if (retCode)
      printf("SATFE_Fsk_Reset error 0x%08X\n", retCode);

   SATFE_RETURN_ERROR("SATFE_Command_fsk_reset()", retCode);
}


/******************************************************************************
 SATFE_Command_fsk_write()
******************************************************************************/
bool SATFE_Command_fsk_write(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[128], i, n;
   BFSK_P_TxParams txParams;

   if (argc < 3)
   {
      SATFE_PrintDescription1("fsk_write", "fsk_write [n] [hex8 ...]", "Sends a 9 byte FSK message in requested TDMA slot.", "n = TDMA slot", false);
      SATFE_PrintDescription2("data comprising entire encapsulated message (without preamble)", true);
      return true;
   }

   if (argc != 11)
   {
      printf("FSK message must be 9 bytes!\n");
      return false;
   }

   n = (uint8_t)atoi(argv[1]);

   for (i = 2; i < argc; i++)
      buf[i-2] = (uint8_t)strtoul(argv[i], NULL, 16);

   txParams.tdmaTxSlot = n;
   SATFE_MUTEX(retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], buf, 9, &txParams));
   if (retCode)
   {
      printf("BFSK_Write() error 0x%X\n", retCode);
      return false;
   }

   SATFE_Ftm_LogWrite(pChip, buf, 9);
   return true;
}


/******************************************************************************
 SATFE_Command_fsk_write()
******************************************************************************/
#define GUNGNIR_TEST_120BYTE_PKT
bool SATFE_Command_fsk_reprogram(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[128], i;
   BFSK_P_TxParams txParams;
#ifdef GUNGNIR_TEST_120BYTE_PKT
   uint32_t total_packets = 0, good_packets = 0;
   unsigned char data_array[117] = {
      0xE2, 0x11, 0xD7, 0x6F, 0x00, 0xCF, 0xFD, 0xF8, 0xFA, 0xE9, 0x71, 0x22, 0x20, 0x80, 0x0F, 0x04,
      0x00, 0xA0, 0xED, 0xCF, 0x70, 0x04, 0x00, 0x10, 0xDB, 0x0A, 0x25, 0x80, 0x0F, 0x00, 0x80, 0x5C,
      0x8A, 0x00, 0x25, 0x82, 0x0F, 0x00, 0x00, 0xFC, 0x03, 0x40, 0xC7, 0x4A, 0x20, 0x00, 0x20, 0x0A,
      0x71, 0x02, 0xDB, 0x98, 0x73, 0x8A, 0x26, 0x04, 0x00, 0x0A, 0x27, 0x00, 0x04, 0x22, 0x20, 0x80,
      0x0F, 0x00, 0x00, 0x64, 0x35, 0x08, 0x77, 0x05, 0xEF, 0xCF, 0x70, 0xFF, 0x00, 0x02, 0x00, 0x4E,
      0xF0, 0x0A, 0x22, 0x80, 0x2F, 0x00, 0x80, 0x34, 0x21, 0x04, 0x1A, 0x00, 0x24, 0x0B, 0xF0, 0xCF,
      0x70, 0x00, 0x80, 0xA0, 0x20, 0x00, 0x80, 0x22, 0x20, 0x80, 0x0F, 0x06, 0x00, 0xDC, 0x2A, 0x08,
      0x76, 0x0A, 0x20, 0x2A, 0x11
   };
   uint8_t n, bytesToRead, nNotRead, retries;
   int8_t key;
#endif

   if (argc != 1)
   {
      SATFE_PrintDescription1("fsk_reprogram", "fsk_reprogram", "Sends a 117 byte FSK message with TDMA disabled.", "n = TDMA slot", true);
      return true;
   }

#ifdef GUNGNIR_TEST_120BYTE_PKT
   while (1) {
   if ((key = SATFE_Platform_GetChar(false)) > 0)
   {
      break;
   }
   for (i = 0; i < 117; i++)
      buf[i] = data_array[i];
#else
   buf[0] = 0xE2;
   buf[1] = 0x11;
   buf[2] = 0xD7; /* write command */

   for (i = 3; i < 117; i++)
      buf[i] = rand() & 0xFF;
#endif

   txParams.tdmaTxSlot = 255;
   SATFE_MUTEX(retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], buf, 117, &txParams));
   if (retCode)
   {
      printf("BFSK_Write() error 0x%X\n", retCode);
      return false;
   }

#ifdef GUNGNIR_TEST_120BYTE_PKT
   i = 0;
   bytesToRead = 9;
   for (retries = 0; retries < 100; retries++)
   {
      SATFE_MUTEX(retCode = BFSK_Read(pChip->hFskChannel[pChip->currFskChannel], &buf[i], bytesToRead, &n, &nNotRead));
      if (retCode)
      {
         printf("BFSK_Read() error 0x%X\n", retCode);
         return false;
      }
      if (n > 0)
      {
         bytesToRead -= n;
         i += n;
         if (bytesToRead == 0)
            break;
      }
      BKNI_Sleep(10);
   }
   if (bytesToRead > 0)
      printf("ERROR: Did not receive response (read %d/9 bytes)\n", i);
   else
   {
      total_packets++;
      if (buf[3] == 0xAA)
         good_packets++;
      else
      {
         if (buf[3] == 0x02)
            printf("Wrong byte at offset %d: read 0x%02X, expected 0x%02X\n", buf[4], buf[5], buf[6]);
         else if (buf[3] == 0x1)
            printf("Wrong number of bytes (%d) received!\n", buf[4]);
         break;
      }
      printf("%d/%d\n", good_packets, total_packets);
   }
   } /* while */
   printf("Summary: %d/%d\n", good_packets, total_packets);
#else
   SATFE_Ftm_LogWrite(pChip, buf, 117);
#endif
   return true;
}


/******************************************************************************
 SATFE_Command_fsk_read()
******************************************************************************/
bool SATFE_Command_fsk_read(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[65], n, nNotRead;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("fsk_read", "fsk_read", "Reads a message from the uC.", "none", true);
      return true;
   }

   SATFE_MUTEX(retCode = BFSK_Read(pChip->hFskChannel[pChip->currFskChannel], buf, 65, &n, &nNotRead));
   if (retCode)
   {
      printf("BFSK_Read() error 0x%X\n", retCode);
      return false;
   }
   if (nNotRead)
   {
      printf("BFSK_Read() %d bytes not read!", nNotRead);
   }

   SATFE_Ftm_LogRead(pChip, buf, n);
   return true;
}


/******************************************************************************
 SATFE_Command_fsk_allocate()
******************************************************************************/
bool SATFE_Command_fsk_allocate(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[9], rf_tuner_id;

   if (argc != 2)
   {
      SATFE_PrintDescription1("fsk_allocate", "fsk_allocate [rf_tuner_id_hex]",
                              "Allocate TDMA slot and user band.",
                              "rf_tuner_id_hex = 0xXX", false);
      SATFE_PrintDescription2("[7:6] 2-bit RF Output Port ID", false);
      SATFE_PrintDescription2("[5:0] 6-bit Tuner ID", true);
      return true;
   }

   if (!bFskInit)
   {
      printf("FSK not initialized!\n");
      return false;
   }

   rf_tuner_id = (uint8_t)strtoul(argv[1], NULL, 16);

   printf("FSK RID = 0x%08X\n", pChip->ftmRid);
   buf[0] = SATFE_FSK_FRAME_REPLY_REQ_FIRST_TX; /* framing - reply required, first transmission */
   buf[1] = SATFE_FSK_ADDR_LNB_DEVICE;          /* address - LNB device */
   buf[2] = SATFE_FSK_CMD_ALLOCATE;             /* command - allocate */
   buf[3] = (uint8_t)((pChip->ftmRid >> 24) & 0xFF);
   buf[4] = (uint8_t)((pChip->ftmRid >> 16) & 0xFF);
   buf[5] = (uint8_t)((pChip->ftmRid >> 8) & 0xFF);
   buf[6] = (uint8_t)(pChip->ftmRid & 0xFF);
   buf[7] = rf_tuner_id;
   buf[8] = 0; /* crc */

   /* TBD timeSlot=0xFF to transmit right away for now */
   if ((retCode = SATFE_Fsk_SendMessage(pChip, buf, 255, buf, 4000, SATFE_FSK_CMD_ALLOCATE)) == BERR_SUCCESS)
      printf("   --> allocated user ID 0x%02X\n", buf[7]);

   SATFE_RETURN_ERROR("SATFE_Command_fsk_allocate()", retCode);
}


/******************************************************************************
 SATFE_Command_fsk_deallocate()
******************************************************************************/
bool SATFE_Command_fsk_deallocate(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[9], user_id;

   if (argc != 2)
   {
      SATFE_PrintDescription1("fsk_deallocate", "fsk_allocate [user_id_hex]",
                              "Deallocate TDMA slot and user band.",
                              "user_id_hex = 0xXX", true);
      return true;
   }

   if (!bFskInit)
   {
      printf("FSK not initialized!\n");
      return false;
   }

   user_id = (uint8_t)strtoul(argv[1], NULL, 16);

   printf("FSK RID = 0x%08X\n", pChip->ftmRid);
   buf[0] = SATFE_FSK_FRAME_REPLY_REQ_FIRST_TX; /* framing - reply required, first transmission */
   buf[1] = SATFE_FSK_ADDR_LNB_DEVICE;          /* address - LNB device */
   buf[2] = SATFE_FSK_CMD_DEALLOCATE;           /* command - deallocate */
   buf[3] = (uint8_t)((pChip->ftmRid >> 24) & 0xFF);
   buf[4] = (uint8_t)((pChip->ftmRid >> 16) & 0xFF);
   buf[5] = (uint8_t)((pChip->ftmRid >> 8) & 0xFF);
   buf[6] = (uint8_t)(pChip->ftmRid & 0xFF);
   buf[7] = user_id;
   buf[8] = 0; /* crc */

   /* TBD timeSlot=0xFF to transmit right away for now */
   if ((retCode = SATFE_Fsk_SendMessage(pChip, buf, 255, buf, 4000, SATFE_FSK_CMD_DEALLOCATE)) == BERR_SUCCESS)
      printf("   --> deallocated tuner ID 0x%02X\n", buf[7]);

   SATFE_RETURN_ERROR("SATFE_Command_fsk_deallocate()", retCode);
}


/******************************************************************************
 SATFE_Command_fsk_odu_reset()
******************************************************************************/
bool SATFE_Command_fsk_odu_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t buf[9];

   if (argc != 1)
   {
      SATFE_PrintDescription1("fsk_odu_reset", "fsk_odu_reset", "Issue global reset to ODU.", "none", true);
      return true;
   }

   if (!bFskInit)
   {
      printf("FSK not initialized!\n");
      return false;
   }

   BKNI_Memset(buf, 0, 9);
   buf[0] = SATFE_FSK_FRAME_REPLY_REQ_FIRST_TX; /* framing - reply required, first transmission */
   buf[1] = SATFE_FSK_ADDR_LNB_DEVICE;          /* address - LNB device */
   buf[2] = SATFE_FSK_CMD_ODU_RESET;            /* command - global reset */

   /* TBD timeSlot=0xFF to transmit right away for now */
   if ((retCode = SATFE_Fsk_SendMessage(pChip, buf, 255, buf, 4000, SATFE_FSK_CMD_ODU_RESET)) == BERR_SUCCESS)
      printf("   --> Resetting ODU...\n", buf[7]);

   SATFE_RETURN_ERROR("SATFE_Command_fsk_odu_reset()", retCode);
}


/******************************************************************************
 bool SATFE_Command_fsk_power_down()
******************************************************************************/
bool SATFE_Command_fsk_power_down(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("fsk_power_down", "fsk_power_down", "Power down FSK core.", "none", true);
      return true;
   }

   retCode = BFSK_PowerDownChannel(pChip->hFskChannel[pChip->currFskChannel]);
   SATFE_RETURN_ERROR("SATFE_Command_fsk_power_down()", retCode);
}


/******************************************************************************
 bool SATFE_Command_fsk_power_up()
******************************************************************************/
bool SATFE_Command_fsk_power_up(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("fsk_power_up", "fsk_power_up", "Power up FSK core.", "none", true);
      return true;
   }

   retCode = BFSK_PowerUpChannel(pChip->hFskChannel[pChip->currFskChannel]);
   SATFE_RETURN_ERROR("SATFE_Command_fsk_power_up()", retCode);
}


/******************************************************************************
 bool SATFE_Command_fsk_carrier()
******************************************************************************/
bool SATFE_Command_fsk_carrier(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bEnable;

   if (argc != 2)
   {
      SATFE_PrintDescription1("fsk_carrier", "fsk_carrier [on | off]", "Enable/disable FSK carrier.", "on = enable FSK carrier", false);
      SATFE_PrintDescription2("off = disable FSK carrier", true);
      return true;
   }

   if (!strcmp(argv[1], "on"))
      bEnable = true;
   else if (!strcmp(argv[1], "off"))
      bEnable = false;
   else
   {
      printf("syntax error\n");
      return false;
   }

   if (bEnable)
   {
      /* enable fsk carrier */
      SATFE_MUTEX(retCode = BFSK_EnableCarrier(pChip->hFskChannel[pChip->currFskChannel]));
   }
   else
   {
      /* disable fsk carrier */
      SATFE_MUTEX(retCode = BFSK_DisableCarrier(pChip->hFskChannel[pChip->currFskChannel]));
   }

   SATFE_RETURN_ERROR("SATFE_Command_fsk_carrier()", retCode);
}


/******************************************************************************
 bool SATFE_Command_config_fsk()
******************************************************************************/
bool SATFE_Command_config_fsk(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bDone = false;
   char c, str[16];
   uint32_t i, tmp, val[BFSK_4538_CHAN_CONFIG_MAX];

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("config_fsk", "config_fsk", "Configure FSK parameters.", "none", true);
      return true;
   }

   while (!bDone)
   {
      for (i = 0; i < BFSK_4538_CHAN_CONFIG_MAX; i++)
      {
         SATFE_MUTEX(retCode = BFSK_GetChannelConfig(pChip->hFskChannel[pChip->currFskChannel], i, &val[i]));
         if (retCode != BERR_SUCCESS)
         {
            printf("BFSK_GetChannelConfig(id=%d) error 0x%X\n", i, retCode);
            return false;
         }
      }

      printf("Select FSK parameter to change:\n");
      printf("a) TX Frequency (Hz)                  : %u\n", val[BFSK_4538_CHAN_CONFIG_TX_FREQ_HZ]);
      printf("b) RX Frequency (Hz)                  : %u\n", val[BFSK_4538_CHAN_CONFIG_RX_FREQ_HZ]);
      printf("c) TX Deviation (Hz)                  : %u\n", val[BFSK_4538_CHAN_CONFIG_TX_DEV_HZ]);
      printf("d) Pre-carrier length (us)            : %u\n", val[BFSK_4538_CHAN_CONFIG_PRECARRIER_US]);
      printf("e) Post-carrier length (us)           : %u\n", val[BFSK_4538_CHAN_CONFIG_POSTCARRIER_US]);
      printf("f) Tx Power                           : %u\n", val[BFSK_4538_CHAN_CONFIG_TX_POWER]);
      tmp = val[BFSK_4538_CHAN_CONFIG_FSK_PREAMBLE];
      printf("g) Preamble pattern                   : 0x%02X %02X %02X %02X\n",
         (tmp >> 24) & 0xFF, (tmp >> 16) & 0xFF, (tmp >> 8) & 0xFF, tmp & 0xFF);
      printf("h) CRC Polynomial                     : 0x%02X\n", val[BFSK_4538_CHAN_CONFIG_CRC_POLY]);
      printf("i) Insert TX CRC                      : %s\n", val[BFSK_4538_CHAN_CONFIG_INSERT_TX_CRC] ? "true" : "false");
      printf("j) Check RX CRC                       : %s\n", val[BFSK_4538_CHAN_CONFIG_CHECK_RX_CRC] ? "true" : "false");
      printf("k) UART baud rate (bits/s)            : %u\n", val[BFSK_4538_CHAN_CONFIG_UART_BAUD_RATE]);
      printf("x) exit\n");
      printf("Selection: ");
      c = SATFE_Platform_GetChar(true);
      fflush(stdin);
      printf("\n\n");

      switch (c)
      {
         case 'a':
            i = BFSK_4538_CHAN_CONFIG_TX_FREQ_HZ;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'b':
            i = BFSK_4538_CHAN_CONFIG_RX_FREQ_HZ;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'c':
            i = BFSK_4538_CHAN_CONFIG_TX_DEV_HZ;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'd':
            i = BFSK_4538_CHAN_CONFIG_PRECARRIER_US;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'e':
            i = BFSK_4538_CHAN_CONFIG_POSTCARRIER_US;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'f':
            i = BFSK_4538_CHAN_CONFIG_TX_POWER;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'g':
            i = BFSK_4538_CHAN_CONFIG_FSK_PREAMBLE;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 16);
            break;
         case 'h':
            i = BFSK_4538_CHAN_CONFIG_CRC_POLY;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 16);
            break;
         case 'i':
            i = BFSK_4538_CHAN_CONFIG_INSERT_TX_CRC;
            val[i] = val[i] ? false : true;
            break;
         case 'j':
            i = BFSK_4538_CHAN_CONFIG_CHECK_RX_CRC;
            val[i] = val[i] ? false : true;
            break;
         case 'k':
            i = BFSK_4538_CHAN_CONFIG_UART_BAUD_RATE;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'x':
            bDone = true;
            break;
         default:
            printf("Invalid selection!\n\n");
            break;
      }

      if ((!bDone) && (i < BFSK_4538_CHAN_CONFIG_MAX))
      {
         SATFE_MUTEX(retCode = BFSK_SetChannelConfig(pChip->hFskChannel[pChip->currFskChannel], i, val[i]));
         if (retCode != BERR_SUCCESS)
         {
            printf("BFSK_SetChannelConfig(id=%d) error 0x%X\n", i, retCode);
            return false;
         }
      }
   }
   return true;
}


/******************************************************************************
 bool SATFE_Command_config_protocol()
******************************************************************************/
bool SATFE_Command_config_protocol(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bDone = false;
   char c, str[16];
   uint32_t i, val[BFSK_PROTOCOL_CONFIG_MAX];

   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("config_protocol", "config_protocol", "Configure FSK debug parameters.", "none", true);
      return true;
   }

   while (!bDone)
   {
      for (i = 0; i < BFSK_PROTOCOL_CONFIG_MAX; i++)
      {
         SATFE_MUTEX(retCode = BFSK_GetChannelConfig(pChip->hFskChannel[pChip->currFskChannel], i+BFSK_PROTOCOL_CONFIG_OFFSET, &val[i]));
         if (retCode != BERR_SUCCESS)
         {
            printf("BFSK_GetChannelConfig(id=%d) error 0x%X\n", i+BFSK_PROTOCOL_CONFIG_OFFSET, retCode);
            return false;
         }
      }

      printf("Select FSK debug parameter to change:\n");
      printf("a) TDMA slot length                   : %u\n", val[BFSK_PROTOCOL_CONFIG_SLOT_LEN]);
      printf("b) TDMA slot count                    : %u\n", val[BFSK_PROTOCOL_CONFIG_SLOT_NUM]);
      printf("c) Enable Tx hold                     : %s\n", val[BFSK_PROTOCOL_CONFIG_ENABLE_TX_HOLD] ? "true" : "false");
      printf("d) Tx delay (us)                      : %u\n", val[BFSK_PROTOCOL_CONFIG_TX_DELAY_US]);
      printf("e) Slotmask for Tx delay              : 0x%08X\n", val[BFSK_PROTOCOL_CONFIG_TX_DELAY_SLOTMASK]);
      printf("f) Request poll count                 : %u\n", val[BFSK_PROTOCOL_CONFIG_REQ_POLL]);
      printf("g) Expected Rx byte count             : %u\n", val[BFSK_PROTOCOL_CONFIG_RX_BYTE_COUNT]);
      printf("x) exit\n");
      printf("Selection: ");
      c = SATFE_Platform_GetChar(true);
      fflush(stdin);
      printf("\n\n");

      switch (c)
      {
         case 'a':
         case 'b':
            printf("Parameter is read-only!\n\n");
            break;
         case 'c':
            i = BFSK_PROTOCOL_CONFIG_ENABLE_TX_HOLD;
            val[i] = val[i] ? false : true;
            break;
         case 'd':
            i = BFSK_PROTOCOL_CONFIG_TX_DELAY_US;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'e':
            i = BFSK_PROTOCOL_CONFIG_TX_DELAY_SLOTMASK;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 16);
            break;
         case 'f':
            i = BFSK_PROTOCOL_CONFIG_REQ_POLL;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'g':
            i = BFSK_PROTOCOL_CONFIG_RX_BYTE_COUNT;
            SATFE_GetString(str);
            val[i] = strtoul(str, NULL, 10);
            break;
         case 'x':
            bDone = true;
            break;
         default:
            printf("Invalid selection!\n\n");
            break;
      }

      if ((!bDone) && (i < BFSK_PROTOCOL_CONFIG_MAX))
      {
         SATFE_MUTEX(retCode = BFSK_SetChannelConfig(pChip->hFskChannel[pChip->currFskChannel], i+BFSK_PROTOCOL_CONFIG_OFFSET, val[i]));
         if (retCode != BERR_SUCCESS)
         {
            printf("BFSK_SetChannelConfig(id=%d) error 0x%X\n", i+BFSK_PROTOCOL_CONFIG_OFFSET, retCode);
            return false;
         }
      }
   }
   return true;
}


/******************************************************************************
 bool SATFE_Command_fsk_test_tdma()
******************************************************************************/
bool SATFE_Command_fsk_test_tdma(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   BFSK_P_TxParams txParams;
   uint32_t n, slotmask, slotnum, currslot, i, val;
   uint8_t j, txBuf[9] = {0xE2, 0x11, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   BSTD_UNUSED(argv);

   if (argc != 3)
   {
      SATFE_PrintDescription1("fsk_test_tdma", "fsk_test_tdma [n] [slotmask]", "Test TDMA Tx repeatedly.",
                              "n = number of iterations", false);
      SATFE_PrintDescription2("slotmask = 0xXXXXX hex bit mask of slots to transmit on", true);
      return true;
   }

   if (!bFskInit)
   {
      printf("FSK not initialized!\n");
      return false;
   }

   n = strtoul(argv[1], NULL, 10);
   slotmask = strtoul(argv[2], NULL, 16);

   /* read num of slots */
   SATFE_MUTEX(retCode = BFSK_GetChannelConfig(pChip->hFskChannel[pChip->currFskChannel],
               BFSK_PROTOCOL_CONFIG_OFFSET+BFSK_PROTOCOL_CONFIG_SLOT_NUM, &val));
   slotnum = val;
   printf("num of slots: %d\n", slotnum);

   for (i = 0; i < n; i++)
   {
      /* hold tdma tx */
      SATFE_MUTEX(retCode = BFSK_SetChannelConfig(pChip->hFskChannel[pChip->currFskChannel],
                  BFSK_PROTOCOL_CONFIG_OFFSET+BFSK_PROTOCOL_CONFIG_ENABLE_TX_HOLD, 1));
      if (retCode)
      {
         printf("BFSK_SetChannelConfig() error 0x%X\n", retCode);
         break;
      }

      /* transmit in slots */
      for (j = 0; j < slotnum; j++)
      {
         currslot = 1 << j;
         if (currslot & slotmask)
         {
            printf("Transmitting in slot %d\n", j);
            txParams.tdmaTxSlot = j;
            SATFE_MUTEX(retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], txBuf, 9, &txParams));
            if (retCode)
            {
               printf("BFSK_Write() error 0x%X\n", retCode);
               break;
            }
         }
      }

      /* start tdma tx */
      SATFE_MUTEX(retCode = BFSK_SetChannelConfig(pChip->hFskChannel[pChip->currFskChannel],
                  BFSK_PROTOCOL_CONFIG_OFFSET+BFSK_PROTOCOL_CONFIG_ENABLE_TX_HOLD, 0));
      if (retCode)
      {
         printf("BFSK_SetChannelConfig() error 0x%X\n", retCode);
         break;
      }

      /* wait at least 1 tdma cycle */
      BKNI_Sleep(200);
   }

   SATFE_RETURN_ERROR("SATFE_Command_fsk_test_tdma()", retCode);
}

#endif
#endif
