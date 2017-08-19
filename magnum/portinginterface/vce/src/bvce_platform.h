/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef BVCE_PLATFORM_H_
#define BVCE_PLATFORM_H_

#include "bint.h"
#include "bvce.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

typedef struct BVCE_P_GetFeatureInfo
{
   unsigned uiDDRDeviceTechCount;
   unsigned uiDramWidthCount;
} BVCE_P_GetFeatureInfo;

/* PLATFORM FLAGS */
#if (BCHP_CHIP == 7425)
#include "bvce_platform_7425.h"
#elif (BCHP_CHIP == 7435)
#include "bvce_platform_7435.h"
#elif (BCHP_CHIP == 7445)
#include "bvce_platform_7445.h"
#elif (BCHP_CHIP == 7145)
#include "bvce_platform_7145.h"
#elif (BCHP_CHIP == 7439)
#include "bvce_platform_7439.h"
#elif (BCHP_CHIP == 7366)
#include "bvce_platform_7366.h"
#elif (BCHP_CHIP == 7278)
#include "bvce_platform_7278.h"
#else
#warning "No Platform Config Specified"
#endif

#define BVCE_PLATFORM_P_MAX_ARC_CORES 2

#ifndef BVCE_PLATFORM_P_NUM_ARC_CORES
#define BVCE_PLATFORM_P_NUM_ARC_CORES BVCE_PLATFORM_P_MAX_ARC_CORES
#endif

#ifndef BVCE_PLATFORM_P_BOX_MODE_SUPPORT
#define BVCE_PLATFORM_P_BOX_MODE_SUPPORT 0
#endif

#ifndef BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS
#error BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS not defined
#endif

#ifndef BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS
#error BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS not defined
#endif

#ifndef BVCE_PLATFORM_P_ITB_ALIGNMENT
#error BVCE_PLATFORM_P_ITB_ALIGNMENT not defined
#endif

#ifndef BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES
#error BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES not defined
#endif

#ifndef MIN_SECURE_BUFFER_SIZE_IN_BYTES
#error MIN_SECURE_BUFFER_SIZE_IN_BYTES not defined
#endif

#ifndef BVCE_PLATFORM_P_SUPPORTS_GROUPAGE
#error BVCE_PLATFORM_P_SUPPORTS_GROUPAGE not defined
#endif

#ifndef CORE_VERSION
#error CORE_VERSION not defined
#endif

#define BVCE_P_MAX_MEMC 3

extern const BVCE_P_GetFeatureInfo BVCE_P_MemcLUT[BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES];
extern const uint32_t BVCE_P_CoreDeviceID[BVCE_PLATFORM_P_MAX_ARC_CORES];

typedef struct BVCE_Platform_P_RegisterSetting
{
      char *szDescription;
      uint32_t uiAddress;
      uint64_t uiValue;
      uint64_t uiMask;
#if BCHP_REGISTER_HAS_64_BIT
      bool bIs64Bit;
#endif
} BVCE_Platform_P_RegisterSetting;

typedef struct BVCE_Platform_P_RegisterSettingsArray
{
      const BVCE_Platform_P_RegisterSetting *astRegisterSettings;
      uint32_t uiRegisterCount;
} BVCE_Platform_P_RegisterSettingsArray;

typedef struct BVCE_Platform_P_Register
{
      uint32_t uiAddress;
      char *szName;
      char *szDescription;
#if BCHP_REGISTER_HAS_64_BIT
      bool bIs64Bit;
#endif
} BVCE_Platform_P_Register;

typedef struct BVCE_Platform_P_RegisterList
{
      signed iInstanceOffset;
      const BVCE_Platform_P_Register *astRegisters;
      uint32_t uiCount;
} BVCE_Platform_P_RegisterList;

typedef struct BVCE_Platform_P_CoreRegisters
{
      uint32_t uiInstructionStartPhysicalAddress;
      uint32_t uiDataSpaceStartRelativeOffset;
      uint32_t uiDataSpaceStartSystemOffset;
      uint32_t uiDCCMBase;
} BVCE_Platform_P_CoreRegisters;

typedef struct BVCE_Platform_P_OutputContext
{
      uint32_t uiReadPointer;
      uint32_t uiBasePointer;
      uint32_t uiValidPointer;
      uint32_t uiWritePointer;
      uint32_t uiEndPointer;
} BVCE_Platform_P_OutputContext;

typedef struct BVCE_Platform_P_OutputRegisters
{
      BVCE_Platform_P_OutputContext stCDB;
      BVCE_Platform_P_OutputContext stITB;
} BVCE_Platform_P_OutputRegisters;

typedef struct BVCE_Platform_P_DebugRegisters
{
      uint32_t uiCMEPictureIndex;
      uint32_t uiArcPC[BVCE_PLATFORM_P_NUM_ARC_CORES];
      uint32_t uiArcHostIF[BVCE_PLATFORM_P_NUM_ARC_CORES];
      uint32_t uiArcHostIFMask[BVCE_PLATFORM_P_NUM_ARC_CORES];
      uint32_t uiPicArcStatus32;
      uint32_t uiCDBDepth[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];
      uint32_t uiSTC[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];
      uint32_t uiScratchRegister;
      uint32_t auiMemcRegBaseLUT[BVCE_P_MAX_MEMC];
      struct
      {
         uint32_t uiBasePointer;
         uint32_t uiEndPointer;
         uint32_t uiReadPointer;
         uint32_t uiWritePointer;
      } stCmd, stBin[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];
} BVCE_Platform_P_DebugRegisters;

typedef struct BVCE_Platform_P_MailboxRegisters
{
      uint32_t uiHost2ViceMailboxAddress;
      uint32_t uiHost2ViceInterruptAddress;
      uint32_t uiHost2ViceInterruptMask;
      uint32_t uiVice2HostMailboxAddress;
      uint32_t uiBvn2ViceMailboxAddress;
} BVCE_Platform_P_MailboxRegisters;

typedef struct BVCE_Platform_P_InterruptMasks
{
      uint32_t uiWatchdog[BVCE_PLATFORM_P_NUM_ARC_CORES];
      uint32_t uiDataReady[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];
      uint32_t uiError;
      uint32_t uiMailbox;
} BVCE_Platform_P_InterruptMasks;

typedef struct BVCE_Platform_P_InterruptSettings
{
      uint32_t uiInterruptStatusRegister;

      BINT_Id idWatchdog[BVCE_PLATFORM_P_NUM_ARC_CORES];
      BINT_Id idDataReady[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];
      BINT_Id idEvent;
      BINT_Id idMailbox;
} BVCE_Platform_P_InterruptSettings;

typedef enum BVCE_Power_Type
{
   BVCE_Power_Type_ePower,
   BVCE_Power_Type_eClock,

   BVCE_Power_Type_eMax
} BVCE_Power_Type;

typedef struct BVCE_Power_P_Resource
{
#ifdef BCHP_PWR_SUPPORT
   BCHP_PWR_ResourceId id;
#else
   unsigned id;
#endif
   unsigned uiRefCount;
} BVCE_Power_P_Resource;

typedef struct BVCE_Platform_P_PowerSettings
{
   BVCE_Power_P_Resource astResource[BVCE_Power_Type_eMax];
   struct
   {
      BVCE_Platform_P_RegisterSetting stSleep;
      BVCE_Platform_P_RegisterSetting stWatchdog;
      BVCE_Platform_P_RegisterSetting stHalt;
   } stCore[BVCE_PLATFORM_P_NUM_ARC_CORES];
} BVCE_Platform_P_PowerSettings;

typedef struct BVCE_Platform_P_BoxModeSettings
{
   unsigned uiTotalInstances;
   unsigned uiTotalChannels;
} BVCE_Platform_P_BoxModeSettings;

typedef struct BVCE_Platform_P_Config
{
      BVCE_Platform_P_RegisterSettingsArray stViceReset;
      BVCE_Platform_P_RegisterSettingsArray stViceWatchdogHandlerEnable;
      BVCE_Platform_P_RegisterSettingsArray stViceWatchdogHandlerDisable;
      BVCE_Platform_P_RegisterSettingsArray stViceBoot;
      BVCE_Platform_P_RegisterSettingsArray stViceWatchdogDisable;

      BVCE_Platform_P_CoreRegisters stCore[BVCE_PLATFORM_P_NUM_ARC_CORES];
      BVCE_Platform_P_OutputRegisters stOutput[BVCE_PLATFORM_P_NUM_OUTPUT_CHANNELS];
      BVCE_Platform_P_InterruptSettings stInterrupt;
      BVCE_Platform_P_DebugRegisters stDebug;
      BVCE_Platform_P_MailboxRegisters stMailbox;
      BVCE_Platform_P_PowerSettings stPower;
      BVCE_Platform_P_BoxModeSettings stBox;

      BVCE_Platform_P_RegisterList stWatchdogRegisterDumpList;
} BVCE_Platform_P_Config;

BERR_Code
BVCE_Platform_P_GetConfig(
         const BBOX_Handle hBox,
         unsigned uiInstance,
         BVCE_Platform_P_Config *pstPlatformConfig
         );

BERR_Code
BVCE_Platform_P_WriteRegisterList(
         BREG_Handle hReg,
         const BVCE_Platform_P_RegisterSetting *astRegisterList,
         uint32_t uiCount
         );

BERR_Code
BVCE_Platform_P_PreReset(
   unsigned uiInstance,
   BREG_Handle hReg
   );

void
BVCE_Platform_P_DumpRegisterList(
         BREG_Handle hReg,
         const BVCE_Platform_P_RegisterList *astRegisterList
         );

#if BVCE_P_ENABLE_UART
BERR_Code
BVCE_Platform_P_EnableUART(
   BREG_Handle hReg
   );
#endif

#if ( BVCE_PLATFORM_P_BOX_MODE_SUPPORT != 0 )
void BVCE_Platform_P_GetTotalInstances(
   BBOX_Handle hBox,
   unsigned *puiTotalInstances
   );

bool
BVCE_Platform_P_IsInstanceSupported(
   const BBOX_Handle hBox,
   unsigned uiInstance
   );

void BVCE_Platform_P_GetTotalChannels(
   BBOX_Handle hBox,
   unsigned uiInstance,
   unsigned *puiTotalChannels
   );

void
BVCE_Platform_P_OverrideChannelDefaultStartEncodeSettings(
   const BBOX_Handle hBox,
   BVCE_Channel_StartEncodeSettings *pstChStartEncodeSettings
   );

void
BVCE_Platform_P_OverrideChannelDefaultEncodeSettings(
   const BBOX_Handle hBox,
   BVCE_Channel_EncodeSettings *pstChEncodeSettings
   );

void
BVCE_Platform_P_OverrideChannelDefaultMemoryBoundsSettings(
   const BBOX_Handle hBox,
   BVCE_Channel_MemoryBoundsSettings *pstChMemoryBoundsSettings
   );

void
BVCE_Platform_P_OverrideChannelDimensionBounds(
   const BBOX_Handle hBox,
   BAVC_ScanType eScanType,
   unsigned *puiWidth,
   unsigned *puiHeight
   );
#else
#define BVCE_Platform_P_GetTotalInstances( _hBox, _puiTotalInstances) { BSTD_UNUSED(_hBox); *(_puiTotalInstances) = BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES; }
#define BVCE_Platform_P_IsInstanceSupported( _hBox, _uiInstance ) (true)
#define BVCE_Platform_P_GetTotalChannels( _hBox, _uiInstance, _puiTotalChannels ) { BSTD_UNUSED(_hBox); BSTD_UNUSED(_uiInstance); *(_puiTotalChannels) = BVCE_PLATFORM_P_NUM_ENCODE_CHANNELS; }
#define BVCE_Platform_P_OverrideChannelDefaultStartEncodeSettings( _hBox, _settings ) { BSTD_UNUSED(_hBox); BSTD_UNUSED(_settings); }
#define BVCE_Platform_P_OverrideChannelDefaultEncodeSettings( _hBox, _settings ) { BSTD_UNUSED(_hBox); BSTD_UNUSED(_settings); }
#define BVCE_Platform_P_OverrideChannelDefaultMemoryBoundsSettings( _hBox, _settings ) { BSTD_UNUSED(_hBox); BSTD_UNUSED(_settings); }
#define BVCE_Platform_P_OverrideChannelDimensionBounds( _hBox, _eScanType, _puiWidth, _puiHeight ) { BSTD_UNUSED(_hBox); BSTD_UNUSED(_eScanType); BSTD_UNUSED(_puiWidth); BSTD_UNUSED(_puiHeight); }
#endif

extern const BVCE_Platform_P_Register s_astViceHardwareRegisters[];
extern const unsigned s_uiViceHardwareRegistersCount;

#ifdef __cplusplus
}
#endif

#endif /* BVCE_PLATFORM_H_ */
