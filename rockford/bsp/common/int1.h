/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   CPU and External Level 1 Interrupt Handler.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _INT1_H
#define _INT1_H

#ifdef MIPS_SDE
    #include "bcmmips.h"
#endif
#include "bchp_common.h"
#include "bchp_hif_cpu_intr1.h"

#if __cplusplus
extern "C" {
#endif

typedef void (*FN_L1_ISR) (void *, int);

#ifdef BCHP_HIF_CPU_INTR1_INTR_W4_STATUS
    #define INT1_MAX_VECTORS 160
#elif BCHP_HIF_CPU_INTR1_INTR_W3_STATUS
    #define INT1_MAX_VECTORS 128
#else
    #define INT1_MAX_VECTORS 96
#endif

typedef struct Int1Control {
    unsigned long IntrW0Status;
    unsigned long IntrW1Status;
    unsigned long IntrW2Status;
#if INT1_MAX_VECTORS >= 128
    unsigned long IntrW3Status;
#endif
#if INT1_MAX_VECTORS >= 160
    unsigned long IntrW4Status;
#endif
    unsigned long IntrW0MaskStatus;
    unsigned long IntrW1MaskStatus;
    unsigned long IntrW2MaskStatus;
#if INT1_MAX_VECTORS >= 128
    unsigned long IntrW3MaskStatus;
#endif
#if INT1_MAX_VECTORS >= 160
    unsigned long IntrW4MaskStatus;
#endif
    unsigned long IntrW0MaskSet;
    unsigned long IntrW1MaskSet;
    unsigned long IntrW2MaskSet;
#if INT1_MAX_VECTORS >= 128
    unsigned long IntrW3MaskSet;
#endif
#if INT1_MAX_VECTORS >= 160
    unsigned long IntrW4MaskSet;
#endif
    unsigned long IntrW0MaskClear;
    unsigned long IntrW1MaskClear;
    unsigned long IntrW2MaskClear;
#if INT1_MAX_VECTORS >= 128
    unsigned long IntrW3MaskClear;
#endif
#if INT1_MAX_VECTORS >= 160
    unsigned long IntrW4MaskClear;
#endif
} Int1Control;                                

/***************************************************************************
 *  BCM7038 Interrupt Level 1 Base Address
 **************************************************************************/
#ifdef MIPS_SDE
    #define CPUINT1C_ADR_BASE BCM_PHYS_TO_K1(BCHP_PHYSICAL_OFFSET+BCHP_HIF_CPU_INTR1_INTR_W0_STATUS)
#else
    #define CPUINT1C_ADR_BASE BCHP_PHYSICAL_OFFSET+BCHP_HIF_CPU_INTR1_INTR_W0_STATUS
#endif

#define CPUINT1C ((Int1Control * const)(CPUINT1C_ADR_BASE))

/***************************************************************************
 *  Level 1 Interrupt Function Prototypes
 **************************************************************************/
extern void CPUINT1_SetInt1Control(Int1Control *int1c);
extern unsigned long CPUINT1_GetInt1ControlAddr(void);
extern void CPUINT1_Isr(void);
extern void CPUINT1_Disable(unsigned long intId);
extern void CPUINT1_Enable(unsigned long intId);
extern int CPUINT1_DisconnectIsr(unsigned long intId);
extern int CPUINT1_ConnectIsr(unsigned long intId, FN_L1_ISR pfunc,
                               void *param1, int param2);

#if __cplusplus
}
#endif

#endif /* _INT1_H */

