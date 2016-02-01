/***************************************************************
 *    (c)2010-2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
****************************************************************/
#if !defined(__OS_SPU_H__)
#define __OS_CPU_H__

#if defined(OS_CPU_GLOBALS)
#define OS_CPU_EXT
#else
#define OS_CPU_EXT extern
#endif

typedef unsigned int BOOLEAN;
typedef unsigned char INT8U;
typedef signed char INT8S;
typedef unsigned short INT16U;
typedef signed short INT16S;
typedef unsigned long INT32U;
typedef signed long INT32S;

/*
typedef float FP32;
typedef double FP64;
*/

typedef unsigned long OS_STK;

typedef unsigned long OS_CPU_SR;
typedef unsigned long register2_t;
typedef void (*exception_handler_t)(register2_t *);
extern OS_CPU_SR OS_CPU_SR_Save(void);
extern void OS_CPU_SR_Restore(OS_CPU_SR);

#define OS_CRITICAL_METHOD 3
#define OS_DISABLE 0
#define OS_ENABLE 0xFF00

#define OS_ENTER_CRITICAL() do { cpu_sr = OS_CPU_SR_Save(); } while(0)
#define OS_EXIT_CRITICAL() do { OS_CPU_SR_Restore(cpu_sr); } while(0)

#define OS_STK_GROWTH 1

#ifdef GHS
#define OS_TASK_SW() do { \
    OS_EXIT_CRITICAL() ; asm("syscall 0,0xb"); } while (0)
#else
#define OS_TASK_SW() do { \
    OS_EXIT_CRITICAL() ; __asm__(".set push; .set mips32; syscall 0xb; .set pop"); } while (0)
#endif

extern void OSIntCtxSw(void);
extern void OSStartHighRdy(void);

extern void OSInitVectors(void);

extern INT8U OSRegisterInterrupt(INT8U vector, void(*handler)(void), void(**old_handler)(void));

extern INT8U OSRegisterException(INT8U vector, exception_handler_t new_handler, exception_handler_t ** old_handler);

#endif
