/******************************************************************************* (c) 2014 Broadcom Corporation
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
#include <ctype.h>
#include <conio.h>
#include <tchar.h>
#include <winsock2.h>
#include <time.h>
#include "satfe.h"
#include "satfe_4538.h"
#include "satfe_platform.h"
//#include "bi2c.h"
#include "bast_4538.h"
#include "bast_4538_priv.h"
#include "stdafx.h"
#include "bchp_leap_host_irq.h"
#include "bchp_leap_ctrl.h"
#include "bhab.h"
#include "bhab_4538.h"
#include "bhab_4538_fw.h"
#include "bhab_4538_priv.h"
#include "bfec.h"
#include "bwfe.h"
#include "bwfe_4538.h"
#ifdef SATFE_USE_BFSK
#include "bfsk_4538.h"
#endif

/*#define SATFE_USE_ISL9494*/


static SATFE_Diags_Config *pConfig;

typedef struct
{
   HANDLE     hIrqEvent;
   HANDLE     hShutdownEvent;
   WORD       wPort;
} THREAD_INFO;


/* global variables */
static THREAD_INFO SATFE_Platform_threadInfo;
HANDLE SATFE_Platform_irqEventThread = NULL;
HANDLE SATFE_Platform_fskEventThread = NULL;
HANDLE SATFE_Platform_shutdownEvent = NULL;
BKNI_MutexHandle SATFE_Platform_hMutex = NULL;
static unsigned long SATFE_Platform_TimerCount = 0;

#ifndef SATFE_USE_ISL9494
#define A8299_0_I2C_ADDR 0x8 /* i2c address of the first A8299 chip */
#define A8299_1_I2C_ADDR 0xB /* i2c address of the second A8299 chip */
uint8_t A8299_control[2] = {0x88, 0x88};
#endif


/* local functions */
BERR_Code SATFE_94538_Open(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_94538_Close(SATFE_Chip *pChip);
BERR_Code SATFE_94538_OpenChannels(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_94538_InitHandle(SATFE_Chip *pChip, void *pParam);
BERR_Code SATFE_94538_Configure(SATFE_Chip *pChip, void *pParam);
DWORD WINAPI SATFE_94538_ProcessInterruptEvents(void* pv);
DWORD WINAPI SATFE_94538_ProcessFskEvents(void* pv);
void SATFE_94538_EnableIrq(bool b, void *p);
void SATFE_94538_LoadCommandFile(char *filename, bool bQuiet);
bool SATFE_94538_Command_constellation(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_diseqc_reset(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_vtop(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_vbot(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_help(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_load(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_plot(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_memory_dump(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_write_isl9494(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_read_isl9494(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_a8299_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_ver(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_program_flash(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_enable_input(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_disable_input(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_standby(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_sa(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_input_status(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_fsk_listen(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_fsk_carrier(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_satcr(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_94538_Command_reset(SATFE_Chip *pChip, int argc, char **argv);

/* define function table for this platform */
static SATFE_PlatformFunctTable SATFE_94538_Functs =
{
   SATFE_94538_Open,          /* open AST/WFE device */
   SATFE_94538_OpenChannels,  /* open AST/WFE channels */
   SATFE_94538_InitHandle,    /* init SATFE_Chip handle */
   SATFE_94538_Configure,     /* chip configuration after AP starts (e.g. LNA, diseqc, etc) */
   NULL,                      /* special handling for TuneAcquire (e.g. external tuner) */
   NULL,                      /* OnIdle */
   SATFE_94538_Close,         /* close AST/WFE device */
};


/* list all Broadcom frontend chips on this board */
SATFE_ChipDescriptor SATFE_chips[SATFE_NUM_CHIPS] =
{
   {
      "BCM4538",     /* name */
      0x4538,      /* chip id */
      BCHP_VER_A0, /* chip version */
      8,           /* number of downstream channels */
      8,           /* number of internal tuners */
      0x68,        /* i2c address */
      &SATFE_94538_Functs,
   }
};


/* define additional commands specific to this platform */
SATFE_Command SATFE_94538_CommandSet[] =
{
   {"load", SATFE_94538_Command_load},
   {"const", SATFE_94538_Command_constellation},
   {"plot", SATFE_94538_Command_plot},
   {"memory_dump", SATFE_94538_Command_memory_dump},
#ifdef SATFE_USE_ISL9494
   {"write_isl9494", SATFE_94538_Command_write_isl9494},
   {"read_isl9494", SATFE_94538_Command_read_isl9494},
#else
   {"a8299_status", SATFE_94538_Command_a8299_status},
#endif
   {"program_flash", SATFE_94538_Command_program_flash},
   {"enable_input", SATFE_94538_Command_enable_input},
   {"disable_input", SATFE_94538_Command_disable_input},
   {"standby", SATFE_94538_Command_standby},
   {"sa", SATFE_94538_Command_sa},
   {"input_status", SATFE_94538_Command_input_status},
   {"fsk_listen", SATFE_94538_Command_fsk_listen},
   {"fsk_carrier", SATFE_94538_Command_fsk_carrier},
   {"satcr", SATFE_94538_Command_satcr},
   {"reset", SATFE_94538_Command_reset},
   {0, NULL},
};


/******************************************************************************
 SoapProc()
******************************************************************************/
#ifdef SOAP
DWORD WINAPI SoapProc(void* p)
{
   extern int SoapMain(int, char **);
   char *argvSoap[2] = {"SoapMain", "--transport:tcp" };

   SoapMain(2, argvSoap);
   return 0;
}
#endif


/******************************************************************************
 SATFE_94538_EnableIrq()
******************************************************************************/
void SATFE_94538_EnableIrq(bool b, void *p)
{
   /* not required with parallel port */
}


/******************************************************************************
 SATFE_94538_Open()
******************************************************************************/
BERR_Code SATFE_94538_Open(SATFE_Chip *pChip, void *pParam)
{
   BERR_Code retCode;
   BAST_Settings astSettings;
   BWFE_Settings wfeSettings;
   BHAB_Settings habSettings;
#ifdef SATFE_USE_BFSK
   BFSK_Settings fskSettings;
#endif
   SATFE_BWFE_Handles *hWfeDev;
   void *pReg;
   static void *context[2];

   pConfig = (SATFE_Diags_Config*)pParam;

   BAST_4538_GetDefaultSettings(&astSettings);
   BWFE_4538_GetDefaultSettings(&wfeSettings);
   BHAB_4538_GetDefaultSettings(&habSettings);
#ifdef SATFE_USE_BFSK
   BFSK_4538_GetDefaultSettings(&fskSettings);
#endif

   habSettings.chipAddr = pChip->chip.addr;
   habSettings.interruptEnableFunc = SATFE_94538_EnableIrq;
   habSettings.interruptEnableFuncParam = (void*)pChip;

   /* open bhab */
   if (pConfig->bSpi)
   {
      habSettings.isSpi = true;
      pReg = pConfig->hRegSpi;
   }
   else
   {
      habSettings.isSpi = false;
      pReg = pConfig->hRegI2c;
   }
   habSettings.pImgInterface = &BHAB_SATFE_IMG_Interface;
   habSettings.pImgContext = (void*)BHAB_4538_IMG_Context;
   retCode = BHAB_Open((BHAB_Handle*)&(pChip->pHab), (void*)pReg, &habSettings);
   if (retCode != BERR_SUCCESS)
   {
      printf("BHAB_Open() error 0x%X\n", retCode);
      goto done;
   }

   /* open bast */
   retCode = BAST_Open(&pChip->hAst, NULL, pChip->pHab, (BINT_Handle)NULL, &astSettings);
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_Open() error 0x%X\n", retCode);
      goto done;
   }

   /* open bwfe */
   pChip->pWfe = (SATFE_BWFE_Handles *)BKNI_Malloc(sizeof(SATFE_BWFE_Handles));
   hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   retCode = BWFE_Open(&(hWfeDev->hWfe), NULL, pChip->pHab, (BINT_Handle)NULL, &wfeSettings);
   if (retCode != BERR_SUCCESS)
   {
      printf("BWFE_Open() error 0x%X\n", retCode);
      goto done;
   }

#ifdef SATFE_USE_BFSK
   retCode = BFSK_Open(&pChip->hFsk, NULL, pChip->pHab, (BINT_Handle)NULL, &fskSettings);
   if (retCode != BERR_SUCCESS)
   {
      printf("BFSK_Open() error 0x%X\n", retCode);
      goto done;
   }
#endif

   done:
   return retCode;
}


/******************************************************************************
 SATFE_94538_Close()
******************************************************************************/
BERR_Code SATFE_94538_Close(SATFE_Chip *pChip)
{
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   int i;

   /* close the BWFE PI */
   if (pChip->pWfe)
   {
      for (i = 0; i < 4; i++)
         BWFE_CloseChannel(hWfeDev->hWfeChannel[i]);

      BWFE_Close(hWfeDev->hWfe);
      pChip->pWfe = NULL;
   }

   BAST_Close(pChip->hAst);

   /* close the BHAB PI */
   if (pChip->pHab)
   {
      BHAB_Close((BHAB_Handle)pChip->pHab);
      pChip->pHab = NULL;
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_94538_OpenChannels()
******************************************************************************/
BERR_Code SATFE_94538_OpenChannels(SATFE_Chip *pChip, void *pParam)
{
   BERR_Code retCode = BERR_SUCCESS;
   BAST_ChannelSettings astChnSettings;
   BWFE_ChannelSettings wfeChnSettings;
   BWFE_ChannelInfo chInfo;
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   uint32_t nAstChannels, i;
   uint8_t j;

   BSTD_UNUSED(pParam);

   BAST_GetTotalChannels(pChip->hAst, &nAstChannels);
   pChip->hAstChannel = (BAST_ChannelHandle *)BKNI_Malloc(nAstChannels * sizeof(BAST_ChannelHandle));

   /* open ast channels */
   for (i = 0; i < nAstChannels; i++)
   {
      BAST_GetChannelDefaultSettings(pChip->hAst, i, &astChnSettings);
      retCode = BAST_OpenChannel(pChip->hAst, &(pChip->hAstChannel[i]), i, &astChnSettings);
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_OpenChannel() error 0x%X\n", retCode);
         BKNI_Free((void*)pChip->hAstChannel);
         break;
      }
   }

   BWFE_GetTotalChannels(hWfeDev->hWfe, &chInfo);
   hWfeDev->hWfeChannel = (BWFE_ChannelHandle *)BKNI_Malloc(chInfo.maxChannels * sizeof(BWFE_ChannelHandle));

   /* open wfe channels */
   for (j = 0; j < chInfo.maxChannels; j++)
   {
      /* skip channel if bonded out */
      if ((chInfo.availChannelsMask >> j) == 0)
      {
         hWfeDev->hWfeChannel[j] = NULL;
         continue;
      }

      BWFE_GetChannelDefaultSettings(hWfeDev->hWfe, j, &wfeChnSettings);
      retCode = BWFE_OpenChannel(hWfeDev->hWfe, &(hWfeDev->hWfeChannel[j]), j, &wfeChnSettings);
      if (retCode != BERR_SUCCESS)
      {
         printf("BWFE_OpenChannel() error 0x%X\n", retCode);
         BKNI_Free((void*)hWfeDev->hWfeChannel);
         break;
      }
   }

   return retCode;
}


/******************************************************************************
 SATFE_94538_InitHandle()
******************************************************************************/
BERR_Code SATFE_94538_InitHandle(SATFE_Chip *pChip, void *pParam)
{
   int i, j;
   pConfig = (SATFE_Diags_Config*)pParam;
   BDBG_ASSERT(pChip->chip.type == 0x4538);
   SATFE_4538_InitHandle(pChip);

   if (SATFE_Platform_hMutex == NULL)
      BKNI_CreateMutex(&SATFE_Platform_hMutex);

   pChip->hMutex = SATFE_Platform_hMutex;
   pChip->hFtmMessageMutex = NULL;
   pChip->pFwImage = pConfig->pFirmware;
   pChip->pPlatformCommandSet = SATFE_94538_CommandSet;
   pChip->pImpl = (SATFE_94538_Impl*)BKNI_Malloc(sizeof(SATFE_94538_Impl));
   BDBG_ASSERT(pChip->pImpl);

   for (i = 0; i < 8; i++)
   {
      ((SATFE_94538_Impl*)(pChip->pImpl))->bConstellation[i] = false;
      ((SATFE_94538_Impl*)(pChip->pImpl))->bPlot[i] = false;
      ((SATFE_94538_Impl*)(pChip->pImpl))->wndConstellation[i] = NULL;
      ((SATFE_94538_Impl*)(pChip->pImpl))->wndPlot[i] = NULL;
      for (j = 0; j < 3; j++)
          ((SATFE_94538_Impl*)(pChip->pImpl))->plotStatus[i][j].item = PLOT_ITEM_NONE;
   }

   /* override any command implementation function */
   pChip->commonCmds.diseqc_reset = SATFE_94538_Command_diseqc_reset;
   pChip->commonCmds.vtop = SATFE_94538_Command_vtop;
   pChip->commonCmds.vbot = SATFE_94538_Command_vbot;
   pChip->commonCmds.help = SATFE_94538_Command_help;
   pChip->commonCmds.ver = SATFE_94538_Command_ver;

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_Platform_Init()
******************************************************************************/
BERR_Code SATFE_Platform_Init(void *pParam)
{
   DWORD dwIrqThreadID, dwFskThreadID;
#ifdef SOAP
   DWORD dwThreadID;
   HANDLE hThread;
#endif
   pConfig = (SATFE_Diags_Config*)pParam;

   SATFE_Platform_threadInfo.hIrqEvent = CreateEvent(NULL, FALSE, FALSE, _T("IRQ_EVENT"));
   SATFE_Platform_threadInfo.hShutdownEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
   SATFE_Platform_threadInfo.wPort = pConfig->lpt_addr;
   SATFE_Platform_shutdownEvent = SATFE_Platform_threadInfo.hShutdownEvent;

   if (SATFE_Platform_irqEventThread == NULL)
   {
      /* create irq processing thread */
     if(pConfig->bSpi == 0)
        SATFE_Platform_irqEventThread = CreateThread( NULL, 0, SATFE_94538_ProcessInterruptEvents, (LPVOID)&SATFE_Platform_threadInfo, 0, &dwIrqThreadID );
#ifdef SOAP
      hThread = CreateThread(NULL, 0, SoapProc, NULL, 0, &dwThreadID);
#endif
   }

   if (SATFE_Platform_fskEventThread == NULL)
   {
      /* create fsk processing thread */
      SATFE_Platform_fskEventThread = CreateThread( NULL, 0, SATFE_94538_ProcessFskEvents, (LPVOID)&SATFE_Platform_threadInfo, 0, &dwFskThreadID );
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 SATFE_Platform_Shutdown()
******************************************************************************/
BERR_Code SATFE_Platform_Shutdown()
{
   SATFE_Chip *pChip;
   int i;

   SetEvent( SATFE_Platform_shutdownEvent );

   if ( SATFE_Platform_irqEventThread != NULL )
      TerminateThread( SATFE_Platform_irqEventThread, -1 );

   if ( SATFE_Platform_fskEventThread != NULL )
      TerminateThread( SATFE_Platform_fskEventThread, -1 );

   BKNI_Sleep(100);
   CloseHandle( SATFE_Platform_irqEventThread );
   CloseHandle( SATFE_Platform_fskEventThread );
   SATFE_Platform_irqEventThread = NULL;
   SATFE_Platform_fskEventThread = NULL;

   for (i = 0; i < SATFE_NUM_CHIPS; i++)
   {
      pChip = SATFE_GetChipByIndex(i);
      if (pChip->pImpl)
      {
         BKNI_Free(pChip->pImpl);
         pChip->pImpl = NULL;
      }
   }

   if (SATFE_Platform_hMutex)
   {
      BKNI_DestroyMutex(SATFE_Platform_hMutex);
      SATFE_Platform_hMutex = NULL;
   }

   return BERR_SUCCESS;
}


#ifdef SATFE_USE_ISL9494
/******************************************************************************
 SATFE_94538_WriteIsl9494()
******************************************************************************/
BERR_Code SATFE_94538_WriteIsl9494(int chip, uint8_t reg, uint8_t data)
{
   BERR_Code retCode;
   uint8_t i2c_addr;

   if (chip == 0)
      i2c_addr = 0x08;
   else
      i2c_addr = 0x09;
   retCode = BREG_I2C_Write((BREG_I2C_Handle)pConfig->hRegI2c, i2c_addr, reg, &data, 1);
   return retCode;
}


/******************************************************************************
 SATFE_94538_ReadIsl9494()
******************************************************************************/
BERR_Code SATFE_94538_ReadIsl9494(int chip, uint8_t reg, uint8_t *pData)
{
   BERR_Code retCode;
   uint8_t i2c_addr;

   if (chip == 0)
      i2c_addr = 0x08;
   else
      i2c_addr = 0x09;
   retCode = BREG_I2C_Read((BREG_I2C_Handle)pConfig->hRegI2c, i2c_addr, reg, pData, 1);
   return retCode;
}


/******************************************************************************
 SATFE_94538_InitIsl() - this function initializes the (2) ISL9494 on 94538 board
******************************************************************************/
BERR_Code SATFE_94538_InitIsl()
{
   BERR_Code retCode = BERR_SUCCESS;
   int i;

   for (i = 0; i < 2; i++)
   {
      SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(i, 0x01, 0x0C));
      if (retCode)
      {
         printf("ISL9494_%d: unable to program Vout1 register\n", i);
         goto done;
      }

      SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(i, 0x02, 0x01));  /* Vout1 register: tone supplied by EXTM pin; 13V */
      if (retCode)
      {
         printf("ISL9494_%d: unable to program Vout1 register\n", i);
         goto done;
      }

      SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(i, 0x04, 0x0C));
      if (retCode)
      {
         printf("ISL9494_%d: unable to program Vout1 register\n", i);
         goto done;
      }

      SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(i, 0x05, 0x01));  /* Vout2 register: tone supplied by EXTM pin; 13V */
      if (retCode)
      {
         printf("ISL9494_%d: unable to program Vout1 register\n", i);
         goto done;
      }
   }

   done:
   if (retCode)
      printf("SATFE_94538_WriteIsl9494() error 0x%X\n", retCode);
   return retCode;
}

#else

/******************************************************************************
 SATFE_94538_A8299_ReadRegister()
******************************************************************************/
BERR_Code SATFE_A8299_ReadRegister(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *pVal)
{
   BERR_Code retCode;
   uint8_t val;

   /* the A8299 doesn't support i2c repeated start condition, so we have to do a write followed by a read */
   val = reg_addr;
   retCode = BREG_I2C_WriteNoAddr((BREG_I2C_Handle)pConfig->hRegI2c, i2c_addr, &val, 1);
   if (retCode)
      goto done;
   retCode = BREG_I2C_ReadNoAddr((BREG_I2C_Handle)pConfig->hRegI2c, i2c_addr, pVal, 1);

   done:
   return retCode;
}


/******************************************************************************
 SATFE_A8299_SetVoltage()
******************************************************************************/
BERR_Code SATFE_A8299_SetVoltage(int channel, bool bVtop)
{
   uint8_t i2c_addr, shift, buf[2], chipIdx, ctl;

   BDBG_ASSERT(channel < 4);

   if (channel >= 2)
   {
      i2c_addr = A8299_1_I2C_ADDR;
      chipIdx = 1;
   }
   else
   {
      i2c_addr = A8299_0_I2C_ADDR;
      chipIdx = 0;
   }

   if ((channel & 1) == 0)
      shift = 0;
   else
      shift = 4;

   ctl = bVtop ? 0xC : 0x8;
   A8299_control[chipIdx] &= ~((0x0F) << shift);
   A8299_control[chipIdx] |= (ctl << shift);
   buf[0] = 0;
   buf[1] = A8299_control[chipIdx];

   /* printf("A8299_%d: channel=%d, i2c_addr=0x%X, ctl=0x%X\n", chipIdx, channel, i2c_addr, buf[1]); */
   return BREG_I2C_WriteNoAddr((BREG_I2C_Handle)pConfig->hRegI2c, i2c_addr, buf, 2);
   //return BREG_I2C_Write((BREG_I2C_Handle)pConfig->hRegI2c, i2c_addr, NULL, buf, 2);
}
#endif


/******************************************************************************
 SATFE_94538_Configure()
******************************************************************************/
BERR_Code SATFE_94538_Configure(SATFE_Chip *pChip, void *pParam)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask;
   uint16_t dsecPinMux;
   uint8_t buf[2];


   /* make GPIO_11, GPIO_12, GPIO_13, and GPIO_14 input */
   mask = (1 << 11) | (1 << 12) | (1 << 13) | (1 << 14);
   SATFE_MUTEX(retCode = BAST_ConfigGpio(pChip->hAst, 0, mask));
   if (retCode)
   {
      printf("BAST_ConfigGpio() error 0x%X\n", retCode);
      return retCode;
   }

   /* configure diseqc pinmuxing */
   SATFE_MUTEX(BAST_ReadConfig(pChip->hAstChannel[0], BAST_4538_CONFIG_DSEC_PIN_MUX, buf, BAST_4538_CONFIG_LEN_DSEC_PIN_MUX));
   dsecPinMux = (buf[0] << 8) | buf[1];
   dsecPinMux |= (BAST_4538_DSEC_PIN_MUX_TXEN_GPIO19 | BAST_4538_DSEC_PIN_MUX_TXOUT_GPIO18); /* enable alternate gpio's */
   buf[0] = (uint8_t)(dsecPinMux >> 8);
   buf[1] = (uint8_t)(dsecPinMux & 0xFF);
   SATFE_MUTEX(BAST_WriteConfig(pChip->hAstChannel[0], BAST_4538_CONFIG_DSEC_PIN_MUX, buf, BAST_4538_CONFIG_LEN_DSEC_PIN_MUX));

   /* diseqc reset is disabled when using the host spi interface */
   if(pConfig->bSpi == 1)
      return retCode;

   /* initialize diseqc */
   return SATFE_94538_Command_diseqc_reset(pChip, 0, NULL);
}


/******************************************************************************
 SATFE_94538_ProcessInterruptEvents()
******************************************************************************/
DWORD WINAPI SATFE_94538_ProcessInterruptEvents( void* pv )
{
   THREAD_INFO* pInfo = (THREAD_INFO*)pv;
   int nInterrupt = -1;
   int nLastInterrupt = -1;
   int nVal;
   WORD wPort = pInfo->wPort;
   BKNI_EventHandle hIrqEvent;
   SATFE_Chip *pChip = SATFE_GetChipByIndex(0); /* only 1 FE chip in this board */

   BHAB_GetInterruptEventHandle((BHAB_Handle)pChip->pHab, &hIrqEvent);

   while ( 1 )
   {
      if ( WaitForSingleObject(SATFE_Platform_shutdownEvent, 1) == WAIT_OBJECT_0 )
         break;

      nVal = _inp((WORD)(wPort + 1));
      nInterrupt = ( nVal & 0x40 ) ? 1 : 0;

      if ((nInterrupt == 1) && (nLastInterrupt == 0))
      {
         BHAB_HandleInterrupt_isr((BHAB_Handle)pChip->pHab);

         while (BKNI_WaitForEvent(hIrqEvent, 0) == BERR_SUCCESS)
         {
            if (BKNI_AcquireMutex(pChip->hMutex) == BERR_SUCCESS)
            {
               BHAB_ProcessInterruptEvent((BHAB_Handle)pChip->pHab);
               BKNI_ReleaseMutex(pChip->hMutex);
            }
         }
      }
      else
         nLastInterrupt = nInterrupt;
   }

   return 0;
}


/******************************************************************************
 SATFE_94538_ProcessFskEvents()
******************************************************************************/
DWORD WINAPI SATFE_94538_ProcessFskEvents( void* pv )
{
   THREAD_INFO* pInfo = (THREAD_INFO*)pv;
   SATFE_Chip *pChip = SATFE_GetChipByIndex(0); /* only 1 FE chip in this board */
   BERR_Code retCode;
   BKNI_EventHandle hFtmEvent;
   uint8_t rxBuf[65], rxLen;
#ifdef SATFE_USE_BFSK
   uint8_t rxBytesUnread;
#endif

#ifdef SATFE_USE_BFSK
   BFSK_GetRxEventHandle(pChip->hFskChannel[0], &hFtmEvent);
#else
   BAST_GetFtmEventHandle(pChip->hAst, &hFtmEvent);
#endif

   while (1)
   {
      if ( WaitForSingleObject(SATFE_Platform_shutdownEvent, 2) == WAIT_OBJECT_0 )
         break;

      if (BKNI_WaitForEvent(hFtmEvent, 10) == BERR_SUCCESS)
      {
         if (hFtmEvent == NULL)
            continue;

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
            printf("BAST_ReadFtm() error 0x%X\n", retCode);
      }
   }

   return 0;
}


/******************************************************************************
 SATFE_Platform_InitDiags()
******************************************************************************/
void SATFE_Platform_InitDiags(void *pParam)
{
   char *init_filename = pConfig->init_filename;
   pConfig = (SATFE_Diags_Config*)pParam;
   SATFE_94538_LoadCommandFile(init_filename, false);
}


/******************************************************************************
 SATFE_LoadCommandFile()
******************************************************************************/
void SATFE_94538_LoadCommandFile(char *filename, bool bQuiet)
{
   FILE *f;
   int argc, line = 0;
   uint32_t i;
   char *argv[64];
   char r[256];

   if (filename[0] == 0)
      return;

   if ((f = fopen(filename, "rt")) == NULL)
   {
      printf("unable to open %s\n", filename);
      return;
   }

   while (!feof(f))
   {
      line++;
      r[0] = 0;
      if (fgets(r, 256, f) == NULL)
         break;

      if (!bQuiet)
        printf("%s", r);

      /* make every character lowercase */
      for (i = 0; i < strlen(r); i++)
      {
         r[i] = tolower(r[i]);
         if (r[i] == '#')
         {
            r[i] = 0;
            break;
         }
      }

      argv[0] = strtok(r, (const char *)" \t\n");
      if (!argv[0])
         continue;

      /* get the rest of the tokens */
      for (argc = 1; ; argc++)
      {
         argv[argc] = strtok((char*)NULL, (const char *)" \t\n");
         if (!argv[argc])
            break;
      }

      SATFE_ProcessCommand(SATFE_GetCurrChip(), argc, argv);
   }
   fclose(f);
}

/******************************************************************************
 SATFE_Platform_GetChar()
******************************************************************************/
char SATFE_Platform_GetChar(bool bBlocking)
{
   char c;

   if (bBlocking)
   {
      do {
        SATFE_OnIdle();
        c = fgetc(stdin);
      } while (c == EOF);
   }
   else
   {
      if (_kbhit())
         c = _getch();
      else
         c = -1;
   }

   SATFE_OnIdle();

   if (c == EOF)
      c = -1;
   else if (c > 0)
      putchar(c);
   return c;
}


/******************************************************************************
 SATFE_Platform_Backspace()
******************************************************************************/
void SATFE_Platform_Backspace()
{
   putchar(' ');
   putchar(0x08);
}


/******************************************************************************
 SATFE_Platform_OnIdle()
******************************************************************************/
void SATFE_Platform_OnIdle()
{
   Sleep(0);
}


/******************************************************************************
 SATFE_Platform_StartTimer()
******************************************************************************/
void SATFE_Platform_StartTimer()
{
   SATFE_Platform_TimerCount = GetTickCount();
}


/******************************************************************************
 SATFE_Platform_KillTimer()
******************************************************************************/
void SATFE_Platform_KillTimer()
{
   SATFE_Platform_TimerCount = 0;
}


/******************************************************************************
 SATFE_Platform_GetTimerCount()
******************************************************************************/
uint32_t SATFE_Platform_GetTimerCount()
{
   if (SATFE_Platform_TimerCount)
      return (GetTickCount() - SATFE_Platform_TimerCount);
   else
      return 0;
}


/******************************************************************************
 SATFE_Platform_Rand()
******************************************************************************/
uint32_t SATFE_Platform_Rand()
{
   static bool bInit = true;
   uint32_t x;

   if (bInit)
   {
      srand((unsigned) time(NULL));
      bInit = false;
   }

   x = (rand() % 256) << 24;
   x |= ((rand() % 256) << 16);
   x |= ((rand() % 256) << 8);
   x |= (rand() % 256);
   return x;
}


/******************************************************************************
 SATFE_Platform_GetInputPower() - rfagc, ifagc, agf, tuner_freq input parameters
    are not used.
******************************************************************************/
//#define SATFE_USE_FLOATING_POINT
void SATFE_Platform_GetInputPower(SATFE_Chip *pChip, uint32_t rfagc,
                                  uint32_t ifagc, uint32_t agf, uint32_t tuner_freq,
                                  float *pPower)
{
   BERR_Code retCode;
   BAST_4538_AgcStatus status;
   uint32_t lna_output_tuning, lna_input_tuning, tI;
#ifdef SATFE_USE_FLOATING_POINT
   double lna_gain, chan_gain, adc_adj;
#else
   int32_t lna_gain, chan_gain, adc_adj;
#endif

   *pPower = 0;

   retCode = BAST_4538_GetAgcStatus(pChip->hAstChannel[pChip->currChannel], &status);
   if (retCode)
   {
      printf("BAST_4538_GetAgcStatus() error 0x%X\n", retCode);
      return;
   }

   lna_output_tuning = status.lnaGain >> 7;
   lna_input_tuning = status.lnaGain & 0x7F;

   /* lna_gain = 32 - (16-output_tuning)*0.5 - (72-inputTuning)*0.5 */
#ifdef SATFE_USE_FLOATING_POINT
   lna_gain = 32 - 0.5 * (88 - lna_output_tuning - lna_input_tuning);
#else
   /* lna_gain is 8.24 format */
   lna_gain = (int32_t)(32 << 24) - (8388608 * (88 - lna_output_tuning - lna_input_tuning));
#endif

   tI = (status.chanAgc >> 8);
   tI ^= (1 << 19);
#ifdef SATFE_USE_FLOATING_POINT
   chan_gain = 20.0 * log10((double)tI / 1024.0);
#else
   /* chan_gain is 8.24 format */
   chan_gain = (BMTH_2560log10(tI) - BMTH_2560log10(1024)) * 131072;
#endif

   switch (status.adcSelect)
   {
      case 0:
      case 1:
#ifdef SATFE_USE_FLOATING_POINT
         adc_adj = 5.0;
#else
         adc_adj = 5 << 24;
#endif
         break;
      case 2:
#ifdef SATFE_USE_FLOATING_POINT
         adc_adj = 4.0;
#else
         adc_adj = 5 << 24;
#endif
         break;
      default: /* ADC3 */
#ifdef SATFE_USE_FLOATING_POINT
         adc_adj = 5.5;
#else
         adc_adj = 92274688; /* 5.5*2^24 */
#endif
         break;
   }

#ifdef SATFE_USE_FLOATING_POINT
   *pPower = (float)(adc_adj - (chan_gain + lna_gain));
#else
   /* need to divide by 2^24 since the terms are in 8.24 format */
   *pPower = (float)((float)(adc_adj - (chan_gain + lna_gain)) / 16777216.0);
#endif
}


/******************************************************************************
 SATFE_94538_Command_constellation()
******************************************************************************/
bool SATFE_94538_Command_constellation(SATFE_Chip *pChip, int argc, char **argv)
{
   extern bool open_constellation_window(SATFE_Chip *pChip);

   if (argc != 1)
   {
      SATFE_PrintDescription1("const", "const", "Display soft decision constellation plot.", "none", true);
      return true;
   }

   return open_constellation_window(pChip);
}


/******************************************************************************
 SATFE_94538_Command_help()
******************************************************************************/
bool SATFE_94538_Command_help(SATFE_Chip *pChip, int argc, char **argv)
{
   printf("PLATFORM-SPECIFIC COMMANDS:\n");
   printf("   load, const, plot, memory_dump, program_flash, enable_input, disable_input,\n");
   printf("   standby, reset\n");
#ifdef SATFE_USE_ISL9494
   printf("   write_isl9494, read_isl9494\n");
#endif
   printf("\n");
   return true;
}


/******************************************************************************
 SATFE_94538_Command_memory_dump()
******************************************************************************/
bool SATFE_94538_Command_memory_dump(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint32_t buf[49152], i, retries, start_addr, end_addr, buf_len;

   if (argc != 4)
   {
      SATFE_PrintDescription1("memory_dump", "memory_dump [start_addr_hex] [end_addr_hex] [filename]", "Reads memory contents into memory_dump.txt file.  This command will stop the LEAP.", "start_addr_hex = starting address in hexadecimal (must be 4-byte aligned)", false);
      SATFE_PrintDescription2("size32_hex = number of 32-bit long words to read in hexadecimal", true);
      return true;
   }

   start_addr = strtoul(argv[1], NULL, 16);
   end_addr = strtoul(argv[2], NULL, 16);
   if ((start_addr & 0x3) || (end_addr & 0x3))
   {
      printf("address is not 4-byte aligned\n");
      return false;
   }
   buf_len = (end_addr - start_addr) >> 2;
   if (buf_len > 49152)
   {
      printf("size too large\n");
      return false;
   }
   printf("stopping the LEAP...\n");
   BKNI_AcquireMutex(pChip->hMutex);
   BHAB_4538_P_Reset((BHAB_Handle)pChip->pHab);

   printf("reading memory from 0x%X to 0x%X (%d words)...\n", start_addr, end_addr, buf_len);
   for (retries = 0; retries < 5; retries++)
   {
      retCode = BHAB_4538_P_ReadRbus((BHAB_Handle)pChip->pHab, start_addr, buf, buf_len);
      if (retCode == BERR_SUCCESS)
         break;
      printf("BAST_4538_ReadRbus() error 0x%X\n", retCode);
   }

   if (retCode == BERR_SUCCESS)
   {
      FILE *fp;

      if ((fp = fopen(argv[3], "w")) == NULL)
      {
         printf("unable to open %s for writing\n", argv[3]);
         goto done;
      }

      for (i = 0; i < buf_len; i++)
         fprintf(fp, "addr 0x%08X: 0x%08X\n", i*4, buf[i]);

      printf("generated memory_dump.txt\n");
      fclose(fp);
   }
   else
      printf("aborting due to errors\n");

   done:
   printf("LEAP is still in reset state\n");
   BKNI_ReleaseMutex(pChip->hMutex);
   return true;
}


/******************************************************************************
 SATFE_94538_Command_diseqc_reset()
******************************************************************************/
bool SATFE_94538_Command_diseqc_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t channel, val;

   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

#ifdef SATFE_USE_ISL9494
   SATFE_RETURN_ERROR("SATFE_94538_InitIsl()", SATFE_94538_InitIsl());
#else
   /* clear any fault condition */
   SATFE_A8299_ReadRegister(A8299_0_I2C_ADDR, 0, &val);
   SATFE_A8299_ReadRegister(A8299_0_I2C_ADDR, 1, &val);
   SATFE_A8299_ReadRegister(A8299_1_I2C_ADDR, 0, &val);
   SATFE_A8299_ReadRegister(A8299_1_I2C_ADDR, 1, &val);

   /* initialize A8299 chips */
   for (channel = 0; channel < 4; channel++)
   {
      retCode = SATFE_A8299_SetVoltage(channel, false);
      if (retCode != BERR_SUCCESS)
         printf("SATFE_A8299_SetVoltage(%d) error 0x%X\n", channel, retCode);
   }

   return true;
#endif
}


/******************************************************************************
 SATFE_94538_Command_ver()
******************************************************************************/
bool SATFE_94538_Command_ver(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   FEC_SystemVersionInfo sysInfo;
   FEC_DeviceVersionInfo devInfo;

   SATFE_MUTEX(retCode = BHAB_GetVersionInfo(pChip->pHab, &sysInfo));
   if (retCode == BERR_SUCCESS)
   {
      printf("Chip family ID   = 0x%X\n", sysInfo.familyId);
      printf("Chip ID          = 0x%X\n", sysInfo.chipId);
      printf("Chip Version     = 0x%X\n", sysInfo.chipVersion);
      printf("Firmware Version = %d.%d Build %d\n", sysInfo.firmware.majorVersion, sysInfo.firmware.minorVersion, sysInfo.firmware.buildId);
   }

   SATFE_MUTEX(retCode = BAST_GetVersionInfo(pChip->hAst, &devInfo));
   if (retCode == BERR_SUCCESS)
   {
      printf("AST PI Version   = %d.%d\n", devInfo.majorVersion, devInfo.minorVersion);
   }

#ifdef SATFE_USE_BFSK
   SATFE_MUTEX(retCode = BFSK_GetVersionInfo(pChip->hFsk, &devInfo));
   if (retCode == BERR_SUCCESS)
   {
      printf("FSK PI Version   = %d.%d\n", devInfo.majorVersion, devInfo.minorVersion);
   }
#endif

   SATFE_RETURN_ERROR("SATFE_94538_Command_ver()", retCode);
}


/******************************************************************************
 SATFE_94538_Command_vtop()
******************************************************************************/
bool SATFE_94538_Command_vtop(SATFE_Chip *pChip, int argc, char **argv)
{
#ifdef SATFE_USE_ISL9494
   BERR_Code retCode;
   int chip = 0;
   uint8_t reg = 0x02;

   switch (pChip->currChannel)
   {
      case 0:
         chip = 0;
         reg = 0x02;
         break;

      case 1:
         chip = 0;
         reg = 0x05;
         break;

      case 2:
         chip = 1;
         reg = 0x05;
         break;

      case 3:
         chip = 1;
         reg = 0x02;
         break;

      default:
         printf("invalid diseqc channel\n");
         return false;
   }

   SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(chip, reg, 0x08));  /* Vout1 register: tone supplied by EXTM pin; 18V */
   SATFE_RETURN_ERROR("SATFE_94538_WriteIsl9494()", retCode);
#else
   SATFE_RETURN_ERROR("SATFE_A8299_SetVoltage()", SATFE_A8299_SetVoltage(pChip->currChannel, true));
#endif
}


/******************************************************************************
 SATFE_94538_Command_vbot()
******************************************************************************/
bool SATFE_94538_Command_vbot(SATFE_Chip *pChip, int argc, char **argv)
{
#ifdef SATFE_USE_ISL9494
   BERR_Code retCode;
   int chip = 0;
   uint8_t reg = 0x02;

   switch (pChip->currChannel)
   {
      case 0:
         chip = 0;
         reg = 0x02;
         break;

      case 1:
         chip = 0;
         reg = 0x05;
         break;

      case 2:
         chip = 1;
         reg = 0x05;
         break;

      case 3:
         chip = 1;
         reg = 0x02;
         break;

      default:
         printf("invalid diseqc channel\n");
         return false;
   }

   SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(chip, reg, 0x01));
   SATFE_RETURN_ERROR("SATFE_94538_WriteIsl9494()", retCode);
#else
   SATFE_RETURN_ERROR("SATFE_A8299_SetVoltage()", SATFE_A8299_SetVoltage(pChip->currChannel, false));
#endif
}


/******************************************************************************
 SATFE_94538_Command_load()
******************************************************************************/
bool SATFE_94538_Command_load(SATFE_Chip *pChip, int argc, char **argv)
{
   char *pFilename = argv[1];
   bool bQuiet = false;

   if ((argc != 2) && (argc != 3))
   {
      SATFE_PrintDescription1("load", "load <-q> [filename]", "Executes the list of commands in the given file.", "-q = quiet mode", false);
      SATFE_PrintDescription2("filename = name of the command file", true);
      return true;
   }

   if (argc == 3)
   {
      if (!strcmp(argv[1], "-q"))
      {
         bQuiet = true;
         pFilename = argv[2];
      }
      else
      {
         printf("syntax error\n");
         return false;
      }
   }

   SATFE_94538_LoadCommandFile(pFilename, bQuiet);
   return true;
}


/******************************************************************************
 SATFE_94538_Command_plot()
******************************************************************************/
bool SATFE_94538_Command_plot(SATFE_Chip *pChip, int argc, char **argv)
{
   extern bool open_plot_window(SATFE_Chip *pChip, int status_item, float min_value, float max_value);

   int item, i;
   float min_value = 0, max_value = 0;
   bool bMin = false, bMax = false;

   if (argc < 2)
   {
      SATFE_PrintDescription1("plot", "plot [status] <-l min> <-h max>", "Plot SNR, power, carrier_error, or BER over time.", "status parameter is one of the following:", false);
      SATFE_PrintDescription2("   snr = Plot SNR", false);
      SATFE_PrintDescription2("   input_power = Plot input power", false);
      SATFE_PrintDescription2("   carrier_error = Plot carrier error", false);
      SATFE_PrintDescription2("   ber = Plot BER", false);
      SATFE_PrintDescription2("   lock = Plot lock status", false);
      SATFE_PrintDescription2("min = minimum value in the plot window", false);
      SATFE_PrintDescription2("max = maximum value in the plot window", true);
      return true;
   }

   if (!strcmp(argv[1], "snr"))
      item = PLOT_ITEM_SNR;
   else if (!strcmp(argv[1], "input_power"))
      item = PLOT_ITEM_INPUT_POWER;
   else if (!strcmp(argv[1], "carrier_error"))
      item = PLOT_ITEM_CARRIER_ERROR;
   else if (!strcmp(argv[1], "lock"))
      item = PLOT_ITEM_LOCK;
   else
   {
      printf("Invalid status item\n");
      return false;
   }

   for (i = 2; i < argc; i++)
   {
      if (!strcmp(argv[i], "-l"))
         bMin = true;
      else if (!strcmp(argv[i], "-h"))
         bMax = true;
      else if (bMin)
      {
         min_value = (float)atof(argv[i]);
         bMin = false;
      }
      else if (bMax)
      {
         max_value = (float)atof(argv[i]);
         bMax = false;
      }
      else
      {
         printf("invalid parameter\n");
         return false;
      }
   }
   return open_plot_window(pChip, item, min_value, max_value);
}


#ifdef SATFE_USE_ISL9494
/******************************************************************************
 SATFE_94538_Command_write_isl9494()
******************************************************************************/
bool SATFE_94538_Command_write_isl9494(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int chip;
   uint8_t reg, val;

   if (argc != 4)
   {
      SATFE_PrintDescription1("write_isl9494", "write_isl9494 [chip] [reg_hex] [val_hex]", "Write to a register on ISL9494.", "chip = selects which ISL9494 to access (0 or 1)", false);
      SATFE_PrintDescription2("   reg_hex = hexadecimal address of the register", false);
      SATFE_PrintDescription2("   val_hex = hexadecimal byte value to set", true);
      return true;
   }

   chip = atoi(argv[1]);
   if ((chip != 0) && (chip != 1))
   {
      printf("invalid chip select\n");
      return false;
   }

   reg = (byte)strtoul(argv[2], NULL, 16);
   if (reg > 0x05)
   {
      printf("invalid register address\n");
      return false;
   }

   val = (byte)strtoul(argv[2], NULL, 16);
   SATFE_MUTEX(retCode = SATFE_94538_WriteIsl9494(chip, reg, val));
   SATFE_RETURN_ERROR("SATFE_94538_WriteIsl9494()", retCode);
}


/******************************************************************************
 SATFE_94538_Command_read_isl9494()
******************************************************************************/
bool SATFE_94538_Command_read_isl9494(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   int chip;
   uint8_t reg, val;

   if ((argc != 3) && (argc != 2))
   {
      SATFE_PrintDescription1("read_isl9494", "read_isl9494 [[[chip] [reg_hex]] | -a]", "Read a register on ISL9494.", "chip = selects which ISL9494 to access (0 or 1)", false);
      SATFE_PrintDescription2("   reg_hex = hexadecimal address of the register", false);
      SATFE_PrintDescription2("   -a = dump all registers", true);
      return true;
   }

   if (argc == 2)
   {
      if (!strcmp(argv[1], "-a"))
      {
         for (chip = 0; chip < 2; chip++)
         {
            printf("ISL9494 #%d:\n", chip);
            for (reg = 0; reg < 6; reg++)
            {
               SATFE_MUTEX(retCode = SATFE_94538_ReadIsl9494(chip, reg, &val));
               if (retCode == BERR_SUCCESS)
                  printf("   Register 0x%02X = 0x%02X\n", reg, val);
               else
                  printf("   error reading register 0x%02X\n", reg);
            }
         }
         return true;
      }
      else
         printf("syntax error\n");
   }

   chip = atoi(argv[1]);
   if ((chip != 0) && (chip != 1))
   {
      printf("invalid chip select\n");
      return false;
   }

   reg = (byte)strtoul(argv[2], NULL, 16);
   if (reg > 0x05)
   {
      printf("invalid register address\n");
      return false;
   }

   SATFE_MUTEX(retCode = SATFE_94538_ReadIsl9494(chip, reg, &val));
   if (retCode == BERR_SUCCESS)
      printf("Register 0x%02X = 0x%02X\n", reg, val);

   SATFE_RETURN_ERROR("SATFE_94538_ReadIsl9494()", retCode);
}

#else

/******************************************************************************
 SATFE_94538_Command_a8299()
******************************************************************************/
bool SATFE_94538_Command_a8299_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t val;

   retCode = SATFE_A8299_ReadRegister(A8299_0_I2C_ADDR, 0, &val);
   if (retCode)
      goto done;
   printf("A8299_0: addr=0, value=0x%X\n", val);
   retCode = SATFE_A8299_ReadRegister(A8299_0_I2C_ADDR, 1, &val);
   if (retCode)
      goto done;
   printf("A8299_0: addr=1, value=0x%X\n", val);
   retCode = SATFE_A8299_ReadRegister(A8299_1_I2C_ADDR, 0, &val);
   if (retCode)
      goto done;
   printf("A8299_1: addr=0, value=0x%X\n", val);
   retCode = SATFE_A8299_ReadRegister(A8299_1_I2C_ADDR, 1, &val);
   if (retCode)
      goto done;
   printf("A8299_1: addr=1, value=0x%X\n", val);

   done:
   SATFE_RETURN_ERROR("BREG_I2C_Read()", retCode);
}
#endif


/* this is needed because BKNI_AssertIsrContext() isn't implemented in win32 KNI */
void BKNI_AssertIsrContext(const char *filename, unsigned lineno)
{
   BSTD_UNUSED(filename);
   BSTD_UNUSED(lineno);
   return;
}


/******************************************************************************
 write_flash_sector() - returns true if successful
******************************************************************************/
bool write_flash_sector(SATFE_Chip *pChip, uint32_t sector_addr, uint8_t *sector_buf, uint32_t sector_buf_addr)
{
#define FLASH_TIMEOUT 300 /* sector programming timeout in 100 msec counts */
   BERR_Code retCode;
   uint32_t i, val32;

   SATFE_MUTEX(retCode = BHAB_WriteMemory((BHAB_Handle)pChip->pHab, sector_buf_addr, sector_buf, 0x1000));
   if (retCode != BERR_SUCCESS)
   {
      printf("BHAB_WriteMemory() error 0x%X\n", retCode);
      return false;
   }

   /* initiate sector programming */
   printf("programming sector 0x%X\n", sector_addr);
   SATFE_MUTEX(retCode = BAST_4538_WriteFlashSector(pChip->hAst, sector_addr));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_4528_WriteFlashSector() error 0x%X\n", retCode);
      return false;
   }

   /* wait for sector to be written */
   for (i = 0; i < FLASH_TIMEOUT; i++)
   {
      BKNI_Sleep(100);
      BHAB_ReadRegister((BHAB_Handle)pChip->pHab, BCHP_LEAP_HOST_IRQ_STATUS0, &val32);
      if (val32 & BHAB_4538_HIRQ0_FLASH_DONE)
      {
         /* clear the interrupt */
         val32 = BHAB_4538_HIRQ0_FLASH_DONE;
         BHAB_WriteRegister((BHAB_Handle)pChip->pHab, BCHP_LEAP_HOST_IRQ_CLEAR0, &val32);

         /* read the flash error status */
         BHAB_ReadRegister((BHAB_Handle)pChip->pHab, BCHP_LEAP_CTRL_SW_SPARE3, &val32);
         val32 = ((val32 & BHAB_4538_SPARE3_FLASH_ERROR_CODE_MASK) >> BHAB_4538_SPARE3_FLASH_ERROR_CODE_SHIFT);
         if (val32)
         {
            printf("flash error 0x%X\n", val32);
            return false;
         }
         break;
      }
   }

   if (i >= FLASH_TIMEOUT)
   {
      printf("timeout waiting for flash sector programming to finish!\n");
      return false;
   }

   return true;
}


/******************************************************************************
 read_flash_file()
******************************************************************************/
bool read_flash_file(char *pFilename, uint8_t **pFlashImage)
{
   FILE *f;
   size_t bytesRead, size, n;
   uint8_t *pFlash, *pData, buf[4];

   if ((f = fopen(pFilename, "rb")) == NULL)
   {
      printf("unable to open %s for reading\n", pFilename);
      return false;
   }

   size = 204 * 1024; /* 204KB */
   pFlash = (uint8_t*)BKNI_Malloc(size);
   BKNI_Memset(pFlash, 0, size);
   *pFlashImage = pFlash;
   pData = pFlash;

   bytesRead = 0;
   do
   {
      n = fread((void*)buf, 1, 4, f);
      pData[bytesRead++] = buf[3];
      pData[bytesRead++] = buf[2];
      pData[bytesRead++] = buf[1];
      pData[bytesRead++] = buf[0];
   } while (n == 4);

   fclose(f);

   if (bytesRead < (203 * 1024)) /* image must be at least 203 KB */
   {
      printf("ERROR! Read only %d bytes\n", bytesRead);
      return false;
   }
   return true;
}


/******************************************************************************
 SATFE_94528_Command_program_flash()
******************************************************************************/
bool SATFE_94538_Command_program_flash(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr, buf_addr;
   uint8_t *pFlash, buf[4];

   if (argc != 2)
   {
      SATFE_PrintDescription1("program_flash", "program_flash [bin_filename]", "Programs the SPI flash.", "filename = Binary file containing flash image", true);
      return true;
   }

   /* read the binary file into pFlash array */
   if (read_flash_file(argv[1], &pFlash) == false)
   {
      printf("read_flash_file() error\n");
      return false;
   }

   /* get the memory address where we'll write the sector data */
   SATFE_MUTEX(retCode = BAST_ReadConfig(pChip->hAstChannel[pChip->currChannel], BAST_4538_CONFIG_FLASH_SECTOR_BUF_ADDR, buf, BAST_4538_CONFIG_LEN_FLASH_SECTOR_BUF_ADDR));
   if (retCode != BERR_SUCCESS)
   {
      printf("BAST_ReadConfig() error 0x%X\n", retCode);
      goto done;
   }
   buf_addr = (uint32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);

   /* write one 4K sector at a time up to 204KB */
   for (addr = 0; addr < 0x33000; addr += 0x1000) /* 204 KB */
   {
      if (write_flash_sector(pChip, addr, &pFlash[addr], buf_addr) == false)
         break;
   }

   done:
   free(pFlash);
   return true;
}


/******************************************************************************
 SATFE_94538_Command_enable_input()
******************************************************************************/
bool SATFE_94538_Command_enable_input(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   BKNI_EventHandle hReadyEvent;
   uint32_t channel;
   bool bEnabled;

   if (argc != 2)
   {
      SATFE_PrintDescription1("enable_input", "enable_input [input]",
                              "enable specified wfe input.",
                              "input: wfe input", true);
      return true;
   }

   channel = strtoul(argv[1], NULL, 10);
   if (channel > 4)
   {
      printf("input must be between 0 - 3\n");
      return false;
   }
   if (hWfeDev->hWfeChannel[channel] == NULL)
   {
      printf("input not available\n");
      return false;
   }

   SATFE_MUTEX(retCode = BWFE_IsInputEnabled(hWfeDev->hWfeChannel[channel], &bEnabled));
   if (retCode != BERR_SUCCESS)
   {
      printf("BWFE_IsInputEnabled() error 0x%X\n", retCode);
      return false;
   }
   if (bEnabled)
      return true;

   retCode = BWFE_GetWfeReadyEventHandle(hWfeDev->hWfeChannel[channel], &hReadyEvent);
   if (retCode)
   {
      printf("BWFE_GetWfeReadyEventHandle() error 0x%X\n", retCode);
      return false;
   }

   BKNI_WaitForEvent(hReadyEvent, 0);
   SATFE_MUTEX(retCode = BWFE_EnableInput(hWfeDev->hWfeChannel[channel]));
   if (retCode)
   {
      printf("BWFE_EnableInput() error 0x%X\n", retCode);
      return false;
   }

#if 0 /*def WIN32 */
   for (int i = 0; i < 5000; i++)
   {
      retCode = BKNI_WaitForEvent(hReadyEvent, 0);
      if (retCode == BERR_SUCCESS)
         break;
      SATFE_OnIdle();
      BKNI_Sleep(1);
      printf(".");
   }
#else
   retCode = BKNI_WaitForEvent(hReadyEvent, 6000);
#endif
   if (retCode)
      printf("timeout waiting for WFE ready\n");

   return true;
}


/******************************************************************************
 SATFE_94538_Command_disable_input()
******************************************************************************/
bool SATFE_94538_Command_disable_input(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   uint32_t channel;

   if (argc != 2)
   {
      SATFE_PrintDescription1("disable_input", "disable_input [input]",
                              "disable specified wfe input.",
                              "input: wfe input", true);
      return true;
   }

   channel = strtoul(argv[1], NULL, 10);
   if (channel > 4)
   {
      printf("input must be between 0 - 3\n");
      return false;
   }
   if (hWfeDev->hWfeChannel[channel] == NULL)
   {
      printf("input not available\n");
      return false;
   }

   SATFE_MUTEX(retCode = BWFE_DisableInput(hWfeDev->hWfeChannel[channel]));
   SATFE_RETURN_ERROR("BWFE_DisableInput()", retCode);
}


/******************************************************************************
 SATFE_94538_Command_input_status()
******************************************************************************/
bool SATFE_94538_Command_input_status(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   uint32_t channel;
   bool bEnabled;

   if (argc != 1)
   {
      SATFE_PrintDescription1("input_status", "input_status",
                              "Query power down status of each ADC.",
                              "none", true);
      return true;
   }

   for (channel = 0; channel < 4; channel++)
   {
      if (hWfeDev->hWfeChannel[channel])
      {
         SATFE_MUTEX(retCode = BWFE_IsInputEnabled(hWfeDev->hWfeChannel[channel], &bEnabled));
         if (retCode != BERR_SUCCESS)
         {
            printf("BWFE_IsInputEnabled() error 0x%X\n", retCode);
            return false;
         }
         printf("ADC%d is powered %s\n", channel, bEnabled ? "on" : "off");
      }
   }

   return true;
}


/******************************************************************************
 bool SATFE_94538_Command_standby()
******************************************************************************/
bool SATFE_94538_Command_standby(SATFE_Chip *pChip, int argc, char **argv)
{
   extern BERR_Code BAST_4538_PowerDownMtsif(BAST_Handle h, bool bPowerDown);
   BERR_Code retCode;
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   uint32_t nChannels, i;
   bool bStandby;

   if (argc != 2)
   {
      SATFE_PrintDescription1("standby", "standby [on | off]", "Enable/Disable chip standby mode.", "on = enter standby mode (power down)", false);
      SATFE_PrintDescription2("off = exit standby mode (power up)", true);
      return true;
   }

   if (!strcmp(argv[1], "on"))
      bStandby = true;
   else
      bStandby = false;

   BAST_GetTotalChannels(pChip->hAst, &nChannels);

   if (bStandby)
   {
      printf("power down FSK...\n");
      SATFE_MUTEX(retCode = BAST_PowerDownFtm(pChip->hAst));
      if (retCode)
      {
         printf("BAST_PowerDownFtm() error 0x%X\n", retCode);
         goto done;
      }

      for (i = 0; i < nChannels; i++)
      {
         /* power down sds, diseqc, channelizer */
         printf("channel %d: power down OIF, DISEQC, channelizer, TFEC, AFEC...\n", i);
         SATFE_MUTEX(retCode = BAST_PowerDown(pChip->hAstChannel[i], BAST_CORE_ALL));
         if (retCode)
         {
            printf("BAST_PowerDown(%d) error 0x%X\n", i, retCode);
            goto done;
         }
      }

#if 0 /* use MXT PI to do this */
      printf("power down MTSIF...\n");
      SATFE_MUTEX(retCode = BAST_4538_PowerDownMtsif(pChip->hAst, true));
#endif

      for (i = 0; i < 4; i++)
      {
         if (hWfeDev->hWfeChannel[i])
         {
            printf("power down WFE%d\n", i);
            SATFE_MUTEX(retCode = BWFE_DisableInput(hWfeDev->hWfeChannel[i]));
            if (retCode)
               printf("BWFE_DisableInput(%d) error 0x%X\n", retCode);
         }
      }

      /* TBD: power down TMT */
      /* TBD: power down Diagnostics RAM */
   }
   else
   {
      for (i = 0; i < 4; i++)
      {
         if (hWfeDev->hWfeChannel[i])
         {
            printf("power up WFE%d\n", i);
            SATFE_MUTEX(retCode = BWFE_EnableInput(hWfeDev->hWfeChannel[i]));
            if (retCode)
               printf("BWFE_EnableInput(%d) error 0x%X\n", retCode);
         }
      }

#if 0 /* use MXT PI to do this */
      printf("power up MTSIF...\n");
      SATFE_MUTEX(retCode = BAST_4538_PowerDownMtsif(pChip->hAst, false));
#endif

      for (i = 0; i < nChannels; i++)
      {
         printf("channel %d: power up OIF, DISEQC, channelizer, SDS...\n", i);
         SATFE_MUTEX(retCode = BAST_PowerUp(pChip->hAstChannel[i], BAST_CORE_ALL));
         if (retCode)
         {
            printf("BAST_PowerDown(%d) error 0x%X\n", i, retCode);
            goto done;
         }
      }

      printf("power up FSK...\n");
      SATFE_MUTEX(retCode = BAST_PowerUpFtm(pChip->hAst));
      if (retCode)
      {
         printf("BAST_PowerUpFtm() error 0x%X\n", retCode);
         goto done;
      }
   }

   done:
   return retCode ? false : true;
}


/******************************************************************************
 SATFE_94538_Command_sa()
******************************************************************************/
bool SATFE_94538_Command_sa(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode = BERR_SUCCESS;
   SATFE_BWFE_Handles *hWfeDev = (SATFE_BWFE_Handles *)pChip->pWfe;
   BKNI_EventHandle hSaEvent;
   BWFE_SpecAnalyzerParams saParams;
   uint32_t i, channel, samples[4096];
   double freq, freqStep;

   if (argc != 5)
   {
      SATFE_PrintDescription1("sa", "sa [input] [start] [stop] [n]",
                              "run spectrum analyzer for wfe input.",
                              "input = wfe input", false);
      SATFE_PrintDescription2("start = start frequency in MHz", false);
      SATFE_PrintDescription2("stop = stop frequency in MHz", false);
      SATFE_PrintDescription2("n = num of samples", true);
      return true;
   }

   channel = strtoul(argv[1], NULL, 10);
   if (channel > 4)
   {
      printf("input must be between 0 - 3");
      return false;
   }
   if (hWfeDev->hWfeChannel[channel] == NULL)
   {
      printf("input not available\n");
      return false;
   }

   saParams.freqStartHz = strtoul(argv[2], NULL, 10) * 1000000;
   saParams.freqStopHz = strtoul(argv[3], NULL, 10) * 1000000;
   if (saParams.freqStartHz >= saParams.freqStopHz)
   {
      printf("freqStart: %d >= freqStop: %d!\n", saParams.freqStartHz, saParams.freqStopHz);
      return false;
   }

   saParams.numSamples = (uint16_t)strtoul(argv[4], NULL, 10);
   if ((saParams.numSamples > 4096) || (saParams.numSamples == 0))
   {
      printf("n must be between 1 and 4096\n");
      return false;
   }

   /* initialize freq step and event handle */
   freq = saParams.freqStartHz;
   freqStep = (double)(saParams.freqStopHz - saParams.freqStartHz) / (saParams.numSamples - 1);
   BWFE_GetSaDoneEventHandle(hWfeDev->hWfe, &hSaEvent);

   printf("Scanning %d Hz to %d Hz (%d samples)...\n", saParams.freqStartHz, saParams.freqStopHz, saParams.numSamples);
   printf("freqStep=%f\n", freqStep);
   SATFE_MUTEX(retCode = BWFE_ScanSpectrum(hWfeDev->hWfeChannel[channel], &saParams));
   if (retCode)
   {
      printf("BWFE_ScanSpectrum error 0x%08X\n", retCode);
      goto done;
   }

   /* wait for sa done*/
   retCode = BKNI_WaitForEvent(hSaEvent, 2000);
   if (retCode == BERR_SUCCESS)
   {
      /* read back data */
      SATFE_MUTEX(retCode = BWFE_GetSaSamples(hWfeDev->hWfe, samples));
      if (retCode)
      {
         printf("BWFE_GetSaSamples error 0x%08X\n", retCode);
         goto done;
      }

      for (i = 0; i < saParams.numSamples; i++)
      {
         printf("%d: 0x%08X -> %.4f\n", (uint32_t)freq, samples[i], samples[i] / 256.0);
         freq = freq + freqStep;
      }
   }
   else if (retCode == BERR_TIMEOUT)
   {
      printf("BKNI_WaitForEvent timed out!\n");
      goto done;
   }
   else
   {
      printf("BKNI_WaitForEvent error 0x%08X!\n", retCode);
      goto done;
   }

   done:
   return retCode ? false : true;
}


/******************************************************************************
 SATFE_94538_Command_fsk_listen()
******************************************************************************/
bool SATFE_94538_Command_fsk_listen(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   uint8_t n;

   if (argc != 2)
   {
      SATFE_PrintDescription1("fsk_listen", "fsk_listen [len]",
                              "listens for message from FSK receiver.",
                              "len = length of packet expected", true);
      return true;
   }

   n = (uint8_t)strtoul(argv[1], NULL, 10);
   SATFE_MUTEX(retCode = BAST_4538_ListenFsk(pChip->hAst, n));

#if 0
   /* TBD wait for time out then reset */
   while (SATFE_Platform_GetChar(false) <= 0)
   {
      SATFE_OnIdle();
   }
#endif

   return retCode ? false : true;
}


/******************************************************************************
 SATFE_94538_Command_fsk_carrier()
******************************************************************************/
bool SATFE_94538_Command_fsk_carrier(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   bool bEnable;

   if (argc != 2)
   {
      SATFE_PrintDescription1("fsk_carrier", "fsk_carrier [on | off]",
                              "enables or disables FSK carrier.",
                              "on = enable FSK carrier", false);
      SATFE_PrintDescription2("off = disable FSK carrier", true);
      return true;
   }

   if (!strcmp(argv[1], "on"))
      bEnable = true;
   else if (!strcmp(argv[1], "off"))
      bEnable = false;
   else
      return false;

   SATFE_MUTEX(retCode = BAST_4538_EnableFskCarrier(pChip->hAst, bEnable));

   return retCode ? false : true;
}

/******************************************************************************
 SATFE_94538_Command_reset()
******************************************************************************/
bool SATFE_94538_Command_reset(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   extern BERR_Code SATFE_Open(void *pParam);
   extern BERR_Code SATFE_Init(void *pParam, bool bInitAp);

   if (argc != 1)
   {
      SATFE_PrintDescription1("reset", "reset", "Reinitialize HAB and AST", "none", true);
      return true;
   }

   printf("Closing all AST and HAB handles...\n");
   SATFE_Shutdown();

   printf("Opening HAB and AST handles...\n");
   retCode = SATFE_Open((void*)pConfig);
   if (retCode != BERR_SUCCESS)
      return 0;

   printf("Downloading FW...\n");
   retCode = SATFE_Init((void*)pConfig, true);
   if (retCode != BERR_SUCCESS)
   {
      printf("Unable to initialize AST PI\n");
      SATFE_Shutdown();
      return 0;
   }

   return true;
}


/******************************************************************************
 SATFE_94538_Command_satcr()
******************************************************************************/
bool SATFE_94538_Command_satcr(SATFE_Chip *pChip, int argc, char **argv)
{
   BERR_Code retCode;
   BAST_DiseqcSettings settings;
   int i, j, n;
   uint32_t rand_delay;
   bool bBypassDC = false, bContinuous = false;
   uint8_t framing;
   char *arg[8], key;
   extern bool SATFE_Command_send(SATFE_Chip *pChip, int argc, char **argv);

   if ((argc < 1) || (argc > 8))
   {
      SATFE_PrintDescription1("satcr", "satcr <-c> <-d> [cmd_hex_bytes ...]", "Send DiSEqC command to the SCIF.", "-d = do not raise DC when transmitting", false);
      SATFE_PrintDescription2("cmd_hex_bytes = 1 to 5 hex bytes (space delimited) to transmit", true);
      return true;
   }

   for (n = 0, i = 1; i < argc; i++)
   {
      if (!strcmp(argv[i], "-c"))
      {
         bContinuous = true;
      }
      else if (!strcmp(argv[i], "-d"))
      {
         bBypassDC = true;
      }
      else
      {
         j = i;
         n = argc - j;
         break;
      }
   }

   if (n <= 0)
   {
      printf("syntax error\n");
      return false;
   }

   while (1)
   {
      if (bContinuous)
      {
         if ((key = SATFE_Platform_GetChar(false)) > 0)
            break;
      }

      if (!bBypassDC)
      {
         /* set DC high */
         arg[0] = "vtop";
         SATFE_94538_Command_vtop(pChip, 1, (char**)arg);
         BKNI_Sleep(9); /* rise time is about 9 msecs */
         BKNI_Sleep(3); /* setup time */
      }

      SATFE_MUTEX(retCode = BAST_GetDiseqcSettings(pChip->hAstChannel[pChip->currChannel], &settings));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_GetDiseqcSettings() error 0x%X\n", retCode);
         return false;
      }

      framing = (uint8_t)strtoul(argv[j], NULL, 16);
      if (((framing >= 0x7A) && (framing <= 0x7E)) || (framing == 0xE2))
      {
         /* reply expected */
         settings.bExpectReply = true;
      }
      else
      {
         /* reply not expected */
         settings.bExpectReply = false;
      }
      settings.bOverrideFraming = true;
      settings.bDisableRxOnly = true;

      SATFE_MUTEX(retCode = BAST_SetDiseqcSettings(pChip->hAstChannel[pChip->currChannel], &settings));
      if (retCode != BERR_SUCCESS)
      {
         printf("BAST_SetDiseqcSettings() error 0x%X\n", retCode);
         return false;
      }

      /* transmit the command */
      arg[0] = "send";
      for (i = 0; i < n; i++)
         arg[i+1] = argv[j+i];
      SATFE_Command_send(pChip, n+1, (char**)arg);

      if (!bBypassDC)
      {
         /* wait time after diseqc message */
         BKNI_Sleep(3);

         /* set DC low */
         arg[0] = "vbot";
         SATFE_94538_Command_vbot(pChip, 1, (char**)arg);
      }

      if (!bContinuous)
         break;

      rand_delay = 500 + rand() % 1000;
      BKNI_Sleep(rand_delay);
   }

   return true;
}