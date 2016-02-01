/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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
#include "int1.h"

/***********************************************************************
 * Local data structures
 ***********************************************************************/
typedef struct
{
    FN_L1_ISR isr;
    void      *param1;
    int       param2;
} Int1Vector;

/***********************************************************************
 * Local #define's
 ***********************************************************************/
#define INT1_GET_REG_NUM(id)	(((id)&0xe0) >> 5)
#define INT1_GET_BIT_NUM(id)	((id)&0x1f)

/***********************************************************************
 * Static variables
 ***********************************************************************/
static Int1Control *CpuInt1Control = CPUINT1C;
static Int1Vector CpuInt1VectorTable[INT1_MAX_VECTORS];

/***********************************************************************
 * void CPUINT1_SetInt1Control()
 *
 * Set the interrupt control structure. Normally it is CPUINT1C.
 * For linux user mode, it should be set to point to a user
 * structure.  If a NULL is passed to this function, it will
 * use set the control structure to CPUINT1C.
 ***********************************************************************/
void CPUINT1_SetInt1Control(Int1Control *int1c)
{
   if (int1c)
      CpuInt1Control = int1c;
   else CpuInt1Control = CPUINT1C;
}

/***********************************************************************
 * void CPUINT1_SetInt1ControlAddr()
 *
 * Get the interrupt control structure address.
 ***********************************************************************/
unsigned long CPUINT1_GetInt1ControlAddr(void)
{
   return (unsigned long) CpuInt1Control;
}

/***********************************************************************
 * void CPUINT1_Isr()
 *
 * Main interrupt handler: Handle all CPU L1 interrupts
 ***********************************************************************/
void CPUINT1_Isr(void)
{
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
    Int1Vector *vec;
    int i;

    #if INT1_MAX_VECTORS >= 160
        if (((IntrW4Status=CpuInt1Control->IntrW4Status)
            & ~(IntrW4MaskStatus=CpuInt1Control->IntrW4MaskStatus)))
        {
            IntrW4Status &= ~IntrW4MaskStatus;
            for (i = 128; i < 160 + 1; i++)
            {
                if (IntrW4Status & 0x1)
                {
                    vec = &(CpuInt1VectorTable[i]);
                    if (vec->isr!=0) {
                        vec->isr(vec->param1, vec->param2);
                    }
                }
                IntrW4Status >>= 1;
            }
        }
    #endif

    #if INT1_MAX_VECTORS >= 128
        if (((IntrW3Status=CpuInt1Control->IntrW3Status)
            & ~(IntrW3MaskStatus=CpuInt1Control->IntrW3MaskStatus)))
        {
            IntrW3Status &= ~IntrW3MaskStatus;

            for (i = 96; i < 128; i++)
            {
                if (IntrW3Status & 0x1)
                {
                    vec = &(CpuInt1VectorTable[i]);
                    if (vec->isr!=0) {
                        vec->isr(vec->param1, vec->param2);
                    }
                }   
                IntrW3Status >>= 1; 
            }
        }
    #endif

    if (((IntrW2Status=CpuInt1Control->IntrW2Status)
        & ~(IntrW2MaskStatus=CpuInt1Control->IntrW2MaskStatus)))
    {
        IntrW2Status &= ~IntrW2MaskStatus;

        for (i = 64; i < 96; i++)
        {
            if (IntrW2Status & 0x1)
            {
                vec = &(CpuInt1VectorTable[i]);
                if (vec->isr!=0) {
                    vec->isr(vec->param1, vec->param2);
                }
            }   
            IntrW2Status >>= 1; 
        }
    }

    if (((IntrW1Status=CpuInt1Control->IntrW1Status)
        & ~(IntrW1MaskStatus=CpuInt1Control->IntrW1MaskStatus)))
    {
        IntrW1Status &= ~IntrW1MaskStatus;

        for (i = 32; i < 64; i++)
        {
            if (IntrW1Status & 0x1)
            {
                vec = &(CpuInt1VectorTable[i]);
                if (vec->isr!=0) {
                    vec->isr(vec->param1, vec->param2);
                }
            }   
            IntrW1Status >>= 1; 
        }
    }

    if (((IntrW0Status=CpuInt1Control->IntrW0Status)
        & ~(IntrW0MaskStatus=CpuInt1Control->IntrW0MaskStatus)))
    {
        IntrW0Status &= ~IntrW0MaskStatus;

        for (i = 0; i < 32; i++)
        {
            if (IntrW0Status & 0x1)
            {
                vec = &(CpuInt1VectorTable[i]);
                if (vec->isr!=0) {
                    vec->isr(vec->param1, vec->param2);
                }
            }   
            IntrW0Status >>= 1; 
        }
    }
}

/***********************************************************************
 * void  CPUINT1_Disable()
 *
 * Disables CPU or external interrupt specified by 'intId'.  Valid
 * intId values/mnemonics can be found in int1.h.
 ***********************************************************************/
void CPUINT1_Disable(unsigned long intId)
{
    unsigned char reg_num = INT1_GET_REG_NUM(intId);
    unsigned char bit_num = INT1_GET_BIT_NUM(intId);

#if INT1_MAX_VECTORS >= 160
    if (reg_num == 4)
        CpuInt1Control->IntrW4MaskSet = 0x1 << bit_num;
    else
#endif
#if INT1_MAX_VECTORS >= 128
    if (reg_num == 3)
        CpuInt1Control->IntrW3MaskSet = 0x1 << bit_num;
    else
#endif
    if (reg_num == 2)
        CpuInt1Control->IntrW2MaskSet = 0x1 << bit_num;
    else if (reg_num == 1)
        CpuInt1Control->IntrW1MaskSet = 0x1 << bit_num;
    else
        CpuInt1Control->IntrW0MaskSet = 0x1 << bit_num;
}


/***********************************************************************
 * void  CPUINT1_Enable()
 *
 * Enables the CPU or external interrupt specified by 'intId'.  Valid
 * intId values/mnemonics can be found in int1.h.
 ***********************************************************************/
void CPUINT1_Enable(unsigned long intId)
{
    unsigned char reg_num = INT1_GET_REG_NUM(intId);
    unsigned char bit_num = INT1_GET_BIT_NUM(intId);

    /* Don't mess with an un-registered interrupt! */   
    if (!CpuInt1VectorTable[intId].isr)
        return;

#if INT1_MAX_VECTORS >= 160
    if (reg_num == 4)
        CpuInt1Control->IntrW4MaskClear = 0x1 << bit_num;
    else
#endif
#if INT1_MAX_VECTORS >= 128
    if (reg_num == 3)
        CpuInt1Control->IntrW3MaskClear = 0x1 << bit_num;
    else
#endif
    if (reg_num == 2)
        CpuInt1Control->IntrW2MaskClear = 0x1 << bit_num;
    else if (reg_num == 1)
        CpuInt1Control->IntrW1MaskClear = 0x1 << bit_num;
    else
        CpuInt1Control->IntrW0MaskClear = 0x1 << bit_num;
}

/***********************************************************************
 * int CPUINT1_ConnectIsr()
 *
 * Maps CPU or external interrupts.  Takes the ISR function pointer
 * 'pfunc' and the parameters 'param1' and 'param2' and stores the
 * values in a vector table, indexed by 'intId'.  'param1' and
 * 'param2' will be passed as parameters to the function 'pfunc'.
 * Valid intId values/mnemonics can be found in int1.h.
 ***********************************************************************/
int CPUINT1_ConnectIsr(unsigned long intId, FN_L1_ISR pfunc,
                       void *param1, int param2)
{
    if (intId > INT1_MAX_VECTORS)
        return 0;

    CpuInt1VectorTable[intId].isr = pfunc;
    CpuInt1VectorTable[intId].param1 = param1;
    CpuInt1VectorTable[intId].param2 = param2;

    return (int) pfunc;
}

/***********************************************************************
 * int CPUINT1_DisconnectIsr()
 *
 ***********************************************************************/
int CPUINT1_DisconnectIsr(unsigned long intId)
{
   if (intId > INT1_MAX_VECTORS)
      return 0;
   
   CpuInt1VectorTable[intId].isr = 0;
   CpuInt1VectorTable[intId].param1 = 0;
   CpuInt1VectorTable[intId].param2 = 0;
   
   return 1;
}

/***********************************************************************
 * void  CPUINT1_GetMaskStatus()
 *
 * Get int1 mask status
 ***********************************************************************/
unsigned long CPUINT1_GetMaskStatus(unsigned long intId)
{
	unsigned char reg_num = INT1_GET_REG_NUM(intId);
	unsigned char bit_num = INT1_GET_BIT_NUM(intId);
	unsigned long status;

    #if INT1_MAX_VECTORS >= 160
        if (reg_num == 4)
            status = CpuInt1Control->IntrW4MaskStatus = (0x1 << bit_num);
        else
    #endif
    #if INT1_MAX_VECTORS >= 128
        if (reg_num == 3)
            status = CpuInt1Control->IntrW3MaskStatus = (0x1 << bit_num);
        else
    #endif
        if (reg_num == 2)
            status = CpuInt1Control->IntrW2MaskStatus & (0x1 << bit_num);
        else if (reg_num == 1)
            status = CpuInt1Control->IntrW1MaskStatus & (0x1 << bit_num);
        else
            status = CpuInt1Control->IntrW0MaskStatus & (0x1 << bit_num);

	return status;
}

