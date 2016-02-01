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
#include "satfe_ftm.h"
#include "bfsk.h"

#ifdef SATFE_USE_BFSK
#ifdef BFSK_PROTOCOL_DTV

bool bFtmInit = false;


/******************************************************************************
 SATFE_Ftm_ResetUc()
******************************************************************************/
BERR_Code SATFE_Ftm_ResetUc(SATFE_Chip *pChip)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t buf[3] = {0x82, 0x07, 0x00};

   /* reset the ftm uC */
   printf("\n*** Resetting FTM uC ***\n");
   SATFE_MUTEX(retCode = BFSK_ResetChannel(pChip->hFskChannel[pChip->currFskChannel]));
   if (retCode != BERR_SUCCESS)
   {
      printf("BFSK_ResetChannel() error 0x%02X, trying local reset command...\n", retCode);
      if ((retCode = SATFE_Ftm_SendMessage(pChip, buf, 3, buf, 3000, 1, 0x07)) != BERR_SUCCESS)
      {
         printf("Failed to get RFmicro reset message! error 0x%02X\n", retCode);
      }
   }
   if (retCode == BERR_SUCCESS)
      bFtmInit = true;
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
 SATFE_Ftm_LogRead()
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
      SATFE_MUTEX(retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], pSendBuf, sendLen, NULL));
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
   BERR_Code retCode;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_uc_reset", "ftm_uc_reset", "Initialize the FTM/FSK block.", "none", true);
      return true;
   }

   retCode = SATFE_Ftm_ResetUc(pChip);
   if (retCode)
   {
      printf("SATFE_Ftm_ResetUc error 0x%08X\n", retCode);
      return false;
   }

   return true;
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

   SATFE_MUTEX(retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], buf, n, NULL));
   if (retCode)
   {
      printf("BFSK_Write() error 0x%X\n", retCode);
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
   uint8_t buf[65], n, nNotRead;
   BSTD_UNUSED(argv);

   if (argc != 1)
   {
      SATFE_PrintDescription1("ftm_read", "ftm_read", "Reads a message from the uC.", "none", true);
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
   }

   buf[0] = 0x81;
   buf[1] = 0x08;

   retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], buf, 2, NULL);
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
   }

   tuner_index = (uint8_t)strtoul(argv[1], NULL, 16);
   if (tuner_index > 15)
   {
      printf("tuner_index out of range\n");
      return BERR_INVALID_PARAMETER;
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
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
      return BERR_INVALID_PARAMETER;
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
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
            return BERR_INVALID_PARAMETER;
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
         return BERR_INVALID_PARAMETER;
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
   }

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 15)
   {
      printf("src_addr out of range\n");
      return BERR_INVALID_PARAMETER;
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
   }

   src_addr = (uint8_t)strtoul(argv[1], NULL, 16);
   if (src_addr > 15)
   {
      printf("src_addr out of range\n");
      return BERR_INVALID_PARAMETER;
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

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
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
   BSTD_UNUSED(pChip);
   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

#if 0
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
      return true;
   }

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
      return false;
   }

   /* register single tuner */
   if (retCode = SATFE_Ftm_Register(pChip, 1) != BERR_SUCCESS)
      return false;

   printf("Press <Enter> to quit\n");
   while (SATFE_GetChar(false) <= 0)
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
#endif
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
      return true;
   }

   if (!bFtmInit)
   {
      printf("FTM not initialized!\n");
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
         return BERR_INVALID_PARAMETER;
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

         retCode = BFSK_Write(pChip->hFskChannel[pChip->currFskChannel], buf, (uint8_t)(payload_len + 7), NULL);
         if (retCode != BERR_SUCCESS)
         {
            printf("BFSK_Write() error 0x%X\n", retCode);
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

   retCode = BFSK_PowerDownChannel(pChip->hFskChannel[pChip->currFskChannel]);
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

   retCode = BFSK_PowerUpChannel(pChip->hFskChannel[pChip->currFskChannel]);
   SATFE_RETURN_ERROR("SATFE_Command_ftm_power_up()", retCode);
}

#endif
#endif
