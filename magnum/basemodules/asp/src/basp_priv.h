/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#ifndef _BASP_PRIV_H_
#define _BASP_PRIV_H_

#include "basp_buffer.h"
#include "basp_msgqueues.h"

#define BASP_P_ALLOCATOR_DEFAULT_ALIGNMENT 1024

#define BASP_P_DEFAULT_FW_MEMORY_ALIGNMENT 1024 /* This is the default alignment settings for BMMA_Allocation. TODO: Later can be changed if not needed to be this big.*/

#define BASP_P_MAX_FIRMWARE_SIZE 128*2*1024 /*TODO: Later based on actual requirement set it to proper size.
                                                    Asper my current understanding,code space is 128K and data space is 128K.*/

#define BASP_P_MAX_INTERFACE_COMM_BUFFER_SIZE 100*1024 /* TODO : Later this will be derived out of bunch of defines each of which defines a buffer size.*/

#define BASP_P_MAX_ASP_SYSTEM_MEMORY_SIZE (BASP_EDPKT_HEADER_BUFFER_SIZE + 1024*5*1024) /* TODO: Once we have actual buffer sizes add them up and compute this size.*/

#define BASP_IDMA_CHANNEL_FIRMWARE_DOWNLOAD 0   /* Use this IDMA channel for firmware code download. */
#define BASP_DDMA_CHANNEL_FIRMWARE_DOWNLOAD 0   /* Use this IDMA channel for firmware data download. */

/* Define indexes into ahCallbacks (callback handle array). */
typedef enum BASP_P_CallbackIndex
{
    BASP_P_CallbackIndex_eWatchdog = 0,
    BASP_P_CallbackIndex_eFw2hRespBit,
    BASP_P_CallbackIndex_eFw2hFinnotifyBit,
    BASP_P_CallbackIndex_eFw2hRstnotifyBit,
    BASP_P_CallbackIndex_eFw2hFincompBit,
    BASP_P_CallbackIndex_eFw2hPaynotifyBit,
    BASP_P_CallbackIndex_eFw2hFrameavailBit,
    BASP_P_CallbackIndex_eFw2hSgfeedcompBit,
    BASP_P_CallbackIndex_eFw2hRtonotifyBit,
    BASP_P_CallbackIndex_eMax
} BASP_P_CallbackIndex;

/*************************************************************************
*  Define some convenience macros.
*
*  Caution!  Beware of side-effects. Don't do BASP_MIN(i++,j++) *
**************************************************************************/
#define BASP_ABS(num)                ((num) < 0 ? -(num) : (num))    /* Returns absolute value of num. */
#define BASP_NEG( num )              ((num) > 0 ? -(num) : (num))    /* Returns negative absolute value of num. */
#define BASP_MIN( x, y )             ((x)<(y) ? (x) : (y))           /* Returns maximum of two numbers. */
#define BASP_MAX( x, y )             ((x)>(y) ? (x) : (y))           /* Returns minimum of two numbers. */
#define BASP_ABSDIFF(x, y)           ((x)>(y) ? ((x)-(y)) : ((y)-(x)))  /* Absolute value of difference.  */
#define BASP_N_ELEMENTS( array )     (sizeof(array)/sizeof(array[0]))

#define   BASP_P_1E9    ((int64_t)1000000000)                         /* 1x10**9  */
#define   BASP_P_1E18U ((uint64_t)1000000000 * 1000000000)   /* Unsigned 1*10**18 */
#define   BASP_P_1E18S ((int64_t)1000000000 * 1000000000)      /* Signed 1*10**18 */

#define  BASP_I64_MAX  ((int64_t)(((uint64_t)-1)>>1))  /* Largest signed 64-bit integer. Equivalent to LLONG_MAX. */
#define  BASP_U64_MAX  ((uint64_t)-1)        /* Largest unsigned 64-bit integer. Equivalent to ULLONG_MAX. */

/* Helper macro for defining a uint64_t constant in a C89 build env.
 * BASP_U64_CONST(12345,678901234)  makes 12345678901234
 *   But be sure to remove any leading zeroes from "low9digits" and "mid9digits" (or C will treat it as octal). */
#define  BASP_I64_CONST(high1digit, mid9digits, low9digits)  (((high1digit)*BASP_P_1E18S) + ((mid9digits)*BASP_P_1E9)+(low9digits))
#define  BASP_U64_CONST(high2digits, mid9digits, low9digits)  (((high2digits)*BASP_P_1E18U) + ((mid9digits)*BASP_P_1E9)+(low9digits))


/* Some macros for printing out 64-bit integers with C89 (because it doesn't have the %lld type of format. */

/* Example:
 *      \* C89 also doesn't have "LL" (64-bit) constants, so  do this to initialize
 *       * u64var to 9223372036854775807 (largest positive signed 64-bit integer): *\
 *      uint64_t    u64var =  9*BASP_P_1E18U +_223372036*BASP_P_1E9 + 854775807) ;
 *      printf( "u64var: "BASP_U64_FMT"\n", BASP_U64_ARG(u64var));
 *
 * Will print: "u64var: 9223372036854775807"
 **/
/* Unsigned versions: */
#define   BASP_U64_FMT  "%.*u%.*u%0*u"
#define   BASP_U64_ARG(num)                                                                                                            \
            (num>=BASP_P_1E18U ? 1 : 0),                         (num>=BASP_P_1E18U ? (uint32_t)(num/BASP_P_1E18U) : 0),               \
            (num>=BASP_P_1E18U ? 9 : (num>=BASP_P_1E9 ? 1 : 0)) , (num>=BASP_P_1E9   ? (uint32_t)((num%BASP_P_1E18U)/BASP_P_1E9) : 0), \
            (num>=BASP_P_1E9   ? 9 : 1) ,                        (num>=BASP_P_1E9   ? (uint32_t)(num%BASP_P_1E9) : (uint32_t)(num))

/* Signed versions: */
#define   BASP_I64_FMT  "%.*s%.*u%.*u%0*u"
#define   BASP_I64_ARG(num)                                                                                                                                                       \
            (num<0 ? 1 : 0),                                                      (num<0 ? "-" : ""),                                                                             \
            (BASP_NEG(num)<=-BASP_P_1E18S ? 1 : 0),                                  (BASP_NEG(num)<=-BASP_P_1E18S ? -(uint32_t)(BASP_NEG(num)/BASP_P_1E18S) : 0),                \
            (BASP_NEG(num)<=-BASP_P_1E18S ? 9 : (BASP_NEG(num)<=-BASP_P_1E9 ? 1 : 0)), (BASP_NEG(num)<=-BASP_P_1E9   ? -(uint32_t)((BASP_NEG(num)%BASP_P_1E18S)/BASP_P_1E9) : 0), \
            (BASP_NEG(num)<=-BASP_P_1E9   ? 9 : 1),                                  (BASP_NEG(num)<=-BASP_P_1E9   ? -(uint32_t)(BASP_NEG(num)%BASP_P_1E9) : -(uint32_t)(BASP_NEG(num)))

/* Hexidecimal versions: */
#define   BASP_X64_FMT  "%.*x%0*x"
#define   BASP_X64_ARG(num)                                                                                           \
            (num>0xffffffff ? 1 : 0),                         (num>0xffffffff ? (uint32_t)(num>>32) : 0),             \
            (num>0xffffffff ? 8 : 1) ,                        (num>0xffffffff ? (uint32_t)(num&0xffffffff) : (uint32_t)(num))

typedef struct BASP_P_Device
{
   BDBG_OBJECT(BASP_P_Device)

   BASP_OpenSettings sOpenSettings;

   struct
   {
         BCHP_Handle hChp;
         BREG_Handle hReg;
         BINT_Handle hInt;
         BTMR_Handle hTmr;
         BMMA_Heap_Handle hMem;
   } handles;

   /* This is for firmware data and code while it is being downloaded by DMA.*/
   struct
   {
      BASP_P_Allocator_Handle hAllocator;
      BASP_P_Buffer_Handle hBuffer;
   }fwMemory;

   /* This is for memory allocated by the host, but only accessed by the ASP (e.g., EdpktHeaderBuffer). */
   struct
   {
      BASP_P_Allocator_Handle hAllocator;
      BASP_P_Buffer_Handle hEdpktHeaderBuffer;
   }aspSystemMemory;

   /* This is for memory accessed by both host and the ASP. */
   struct
   {
      BASP_P_Allocator_Handle hAllocator;
      BASP_P_Buffer_Handle hHwInitInfoBuffer; /* This buffer will be based on the hwinit info binary file that will be provided by fw team */
      BASP_P_Buffer_Handle hDebugBuffer;
   }aspCommMemory;

   /* Here is an array of the various BINT callback handles. */
   struct
   {
         BINT_CallbackHandle ahCallbacks[BASP_P_CallbackIndex_eMax];
   } callbacks;

   BAFL_FirmwareLoadInfo   sFirmwareLoadInfo;

   BLST_S_HEAD(contextList,  BASP_P_Context)  contextList;
   BLST_S_HEAD(msgqueueList, BASP_P_Msgqueue) msgqueueList;

   BASP_MsgqueueHandle   hMsgqueueFwToHost;
   BASP_MsgqueueHandle   hMsgqueueHostToFw;
   BASP_MsgqueueHandle   hMsgqueueRaToFw;
   BASP_MsgqueueHandle   hMsgqueueFwToRa;

   struct
   {
       uint32_t     cachedAddress;      /* Address of the word in the cache. */
       uint32_t     cachedValue;        /* Value of the what's in cachedAddress. */

       BTMR_TimerHandle hTimer;

   } DBG_HostPrintf;

} BASP_P_Device;

typedef struct BASP_P_Context
{
   BDBG_OBJECT(BASP_P_Context)
   BASP_ContextCreateSettings sCreateSettings;
   BASP_Handle hAsp;
   BLST_S_ENTRY(BASP_P_Context) nextContext;
   BLST_S_HEAD(channelList,  BASP_P_Channel)  channelList;

} BASP_P_Context;

typedef struct BASP_P_Channel
{
   BDBG_OBJECT(BASP_P_Channel)
   BASP_ChannelCreateSettings sCreateSettings;
   BASP_ChannelCallbacks callbacks;
   BASP_ContextHandle hContext;
   BLST_S_ENTRY(BASP_P_Channel) nextChannel;
   uint32_t channelIndex;

} BASP_P_Channel;


BASP_ChannelHandle
BASP_P_Channel_GetByChannelIndex_isr(
    BASP_Handle  hAsp,
    uint32_t channelIndex
    );

void BASP_P_Channel_FireMessageReadyCallback_isr(
    BASP_ChannelHandle  hChannel
    );

BERR_Code
BASP_P_AllocateDeviceMemory(
         BASP_P_Allocator_Handle hAllocator,
         BASP_P_Buffer_Handle    *phBuffer,
         uint32_t                ui32BufferSizeInBytes,
         uint32_t                ui32Allignment
         );

void
BASP_P_FreeDeviceMemory(
        BASP_P_Buffer_Handle hBuffer
        );

void
BASP_P_DestroyAllocators(
   BASP_Handle hAsp
   );

BERR_Code
BASP_P_SetupAllocators(
   BASP_Handle hAsp
   );

BERR_Code
BASP_P_Reset(
   BASP_Handle hAsp
   );

BERR_Code
BASP_P_LoadFirmware(
   BASP_Handle hAsp
   );

BERR_Code
BASP_P_FreeHwInitInfo(
   BASP_Handle hAsp
   );

BERR_Code
BASP_P_LoadHwInitInfo(
   BASP_Handle hAsp
   );

BERR_Code
BASP_P_CheckHwInitInfo(
    BASP_Handle hAsp
    );


#endif /* _BASP_PRIV_H_ */
