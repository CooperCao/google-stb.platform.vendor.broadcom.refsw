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

#ifndef _SATFE_H_
#define _SATFE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bast.h"
#include "bkni.h"
#include "bkni_multi.h"
#ifdef SATFE_USE_BFSK
#include "bfsk.h"
#endif


typedef enum
{
   SATFE_RegisterType_eVirtual = 0,
   SATFE_RegisterType_eISB,
   SATFE_RegisterType_eHost,
   SATFE_RegisterType_eIndirect,
   SATFE_RegisterType_eExternal
} SATFE_RegisterType;


typedef struct
{
   char               *name;
   uint32_t           addr;
   SATFE_RegisterType type;
} SATFE_Register;


typedef enum
{
   SATFE_ConfigParamType_eDefault = 0,
   SATFE_ConfigParamType_eReadOnly,
   SATFE_ConfigParamType_eHidden,
   SATFE_ConfigParamType_eAmcScrambling
} SATFE_ConfigParamType;


typedef struct
{
   char                  *name;
   uint16_t              id;
   uint8_t               len;
   SATFE_ConfigParamType type;
   char                  *desc;
} SATFE_ConfigParam;


struct SATFE_Chip;


typedef bool (*SATFE_CommandFunct)(struct SATFE_Chip*, int, char **);


typedef struct
{
   char *name;
   SATFE_CommandFunct funct;
} SATFE_Command;


typedef struct
{
   uint32_t xseed;
   uint32_t plhdrscr1;
   uint32_t plhdrscr2;
   uint32_t plhdrscr3;
} SATFE_AmcScramblingSeq;
extern SATFE_AmcScramblingSeq SATFE_ScramblingSeqList[];


typedef struct
{
   BERR_Code (*Open)(struct SATFE_Chip*, void *pParam);
   BERR_Code (*OpenChannels)(struct SATFE_Chip*, void *pParam);
   BERR_Code (*InitHandle)(struct SATFE_Chip*, void *pParam);
   BERR_Code (*Configure)(struct SATFE_Chip*, void *pParam);
   BERR_Code (*TuneAcquire)(struct SATFE_Chip*, uint32_t freq, BAST_AcqSettings *pParams);
   void      (*OnIdle)(void);
   BERR_Code (*Close)(struct SATFE_Chip*);
} SATFE_PlatformFunctTable;


typedef struct
{
   char     *name;   /* if NULL, name will be set to chip name */
   uint16_t type;    /* chip number in hex, e.g. 0x4506 for BCM4506 */
   uint32_t version; /* BCHP_VER_* */
   uint8_t  nDemods; /* number of downstream channels */
   uint8_t  nTuners; /* number of internal tuners */
   uint8_t  addr;    /* i2c address (if applicable) */
   SATFE_PlatformFunctTable *platformFunctTable;
} SATFE_ChipDescriptor;


typedef struct
{
   SATFE_CommandFunct ta;
   SATFE_CommandFunct abort;
   SATFE_CommandFunct status;
   SATFE_CommandFunct ver;
   SATFE_CommandFunct help;
   SATFE_CommandFunct monitor;
   SATFE_CommandFunct diseqc_reset;
   SATFE_CommandFunct vtop;
   SATFE_CommandFunct vbot;
   SATFE_CommandFunct tone;
   SATFE_CommandFunct send;
   SATFE_CommandFunct vsense;
   SATFE_CommandFunct monitor_lock_events;
} SATFE_CommandMap;


typedef struct
{
   bool (*GetFreqFromString)(char *str, uint32_t *pHz);
   bool (*IsSpinv)(struct SATFE_Chip *pChip, BAST_ChannelStatus *pStatus);
   bool (*IsPilotOn)(struct SATFE_Chip *pChip, BAST_ChannelStatus *pStatus);
   void (*GetDebugModuleNames)(struct SATFE_Chip *pChip, int *numModules, char **moduleNames[]);
   bool (*GetAdcVoltage)(struct SATFE_Chip *pChip, uint8_t voltage, uint8_t currChannel, uint16_t *lnbVoltage);
   bool (*Mi2cWrite)(struct SATFE_Chip *pChip, uint8_t channel, uint8_t slave_addr, uint8_t *buf, uint8_t n);
   BERR_Code (*Mi2cRead)(struct SATFE_Chip *pChip, uint8_t channel, uint8_t slave_addr, uint8_t *out_buf, uint8_t out_n, uint8_t *in_buf, uint8_t in_n);
} SATFE_ChipFunctTable;


typedef struct SATFE_Chip
{
   int                  idx;
   int                  currChannel;
   int                  currAcqSettings;
   int                  currFskChannel;
   SATFE_ChipDescriptor chip;
   SATFE_Register       *regMap;
   SATFE_ConfigParam    *configParamMap;
   SATFE_CommandMap     commonCmds;
   SATFE_Command        *pPlatformCommandSet;
   SATFE_Command        *pChipCommandSet;
   SATFE_ChipFunctTable *chipFunctTable;
   BAST_Handle          hAst;
   BAST_ChannelHandle   *hAstChannel;
#ifdef SATFE_USE_BFSK
   BFSK_Handle          hFsk;
   BFSK_ChannelHandle   *hFskChannel;
   uint32_t             nFskChannels;
#endif
   BKNI_MutexHandle     hMutex;
   BKNI_MutexHandle     hFtmMessageMutex;
   BKNI_EventHandle     hFtmMessageEvent;
   BAST_AcqSettings     *acqSettings;
   uint32_t             ftmRid;
   uint32_t             ftmPacketCount;
   uint32_t             amcScramblingIdx[2];
   void                 *pImpl;
   void                 *pHab;   /* used only if chip has BHAB PI */
   void                 *pWfe;   /* used only if chip has BWFE PI */
   void                 *pMxt;   /* used only if chip has MXT PI */
   uint8_t              *pFwImage;
   bool                 bInit;
   bool                 bEnableFtmLogging;
   uint8_t              ftmMessageBuf[65];
   uint8_t              ftmTunerRegAddr[8];
} SATFE_Chip;


typedef struct
{
   BERR_Code error_code;
   char      *description;
} SATFE_ErrorDesc;
extern SATFE_ErrorDesc SATFE_ErrorList[];


typedef struct
{
   char      *name;
   BAST_Mode mode;
} SATFE_ModeDesc;
extern SATFE_ModeDesc SATFE_ModeList[];


#define SATFE_CHECK_RETCODE(m,x) \
   if ((x) != BERR_SUCCESS) { \
      printf(#m " error 0x%X\n", x); \
      return (bool)(x); }

#define SATFE_RETURN_ERROR(m,x) \
   if ((x) != BERR_SUCCESS) { \
      printf(#m " error 0x%X\n", x); \
      return false; \
   } else return true;

#define SATFE_GOTO_DONE_ON_ERROR(m,x) \
   if ((x) != BERR_SUCCESS) { \
      printf(#m " error 0x%X\n", x); \
      goto done; }

#define SATFE_DO_RETRY(x) for (retry = 0; retry < 5; retry++) { x; if (retCode == BERR_SUCCESS) break; }

SATFE_Chip* SATFE_GetCurrChip(void);
SATFE_Chip* SATFE_GetChipByIndex(int idx);
bool SATFE_LookupCommand(SATFE_Chip *pChip, char *cmdstr, SATFE_CommandFunct *funct);
bool SATFE_LookupRegister(SATFE_Chip *pChip, char *name, SATFE_Register **pReg);
void SATFE_ProcessCommand(SATFE_Chip *pChip, int argc, char **argv);
bool SATFE_IsFtmInit(SATFE_Chip*);
void SATFE_PrintDescription1(char *cmd, char *syntax, char *desc, char *option, bool bEnd);
void SATFE_PrintDescription2(char *option, bool bEnd);
bool SATFE_GetU32FromString(char *str, uint32_t *pVal);
bool SATFE_GetU16FromString(char *str, uint16_t *pVal);
bool SATFE_GetU8FromString(char *str, uint8_t *pVal);
uint8_t SATFE_GetStreamCrc8(uint8_t *pBuf, int n);
void SATFE_TimedAcquire(SATFE_Chip *pChip, uint32_t tuner_freq, BAST_AcqSettings *pSettings, uint32_t lockTimeout, uint32_t lockStableTime, bool *pAborted, bool *pLocked, BAST_ChannelStatus *pStatus, uint32_t *pAcqTime);
bool SATFE_GetFreqFromString(SATFE_Chip *pChip, char *str, uint32_t *pHz);
char* SATFE_GetModeString(BAST_Mode mode);
void SATFE_GetString(char *cmdline);
bool SATFE_WriteAmcScramblingSeq(SATFE_Chip *pChip, int idx);
bool SATFE_LookupRegister(SATFE_Chip *pChip, char *name, SATFE_Register **pReg);
void SATFE_OnIdle(void);
char* SATFE_get_32bit_binary_string(uint32_t val32);

int SATFE_Diags(void *pParam, int bReset);
BERR_Code SATFE_Shutdown(void);
bool SATFE_GetSoftDecisions(SATFE_Chip *pChip, int channel, short *iBuf, short *qBuf);
bool SATFE_GetStatusItem(SATFE_Chip *pChip, int chn, int statusItem, void *pStatus);
void SATFE_Ftm_LogRead(SATFE_Chip *pChip, uint8_t *buf, uint8_t len);

#define SATFE_STATUS_SNR           0
#define SATFE_STATUS_INPUT_POWER   1
#define SATFE_STATUS_CARRIER_ERROR 2
#define SATFE_STATUS_LOCK          3

#ifdef __cplusplus
};
#endif

#endif
