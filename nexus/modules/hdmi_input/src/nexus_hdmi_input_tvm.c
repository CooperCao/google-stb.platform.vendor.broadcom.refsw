/******************************************************************************
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
 *****************************************************************************/



#include "nexus_hdmi_input_module.h"
#include "bchp_tvm.h"
#include "bchp_hdmi_rx_fe_0.h"


void NEXUS_HdmiInputTvm_P_HotPlugEnable(void)
{
    uint32_t lval;
    BREG_Handle hReg = g_pCoreHandles->reg;

#if (BCHP_CHIP==35230)
    lval = BREG_Read32(hReg, BCHP_TVM_PAD_CTRL_4);

    lval &= ~(BCHP_TVM_PAD_CTRL_4_FORCE_HP_OUT_HIGH_MASK | 
                   BCHP_TVM_PAD_CTRL_4_FORCE_HP_OUT_LOW_MASK |
                       BCHP_TVM_PAD_CTRL_4_HOTPLUG_OUT_PAD_OEb_MASK);
    lval |= (BCHP_TVM_PAD_CTRL_4_EN_DVP_HR_HOTPLUG_IN_MASK | 
                BCHP_TVM_PAD_CTRL_4_EN_DVP_HR_HOTPLUG_OUT_MASK);

    BREG_Write32(hReg, BCHP_TVM_PAD_CTRL_4, lval);
#elif (BCHP_CHIP == 35125)
    lval = BREG_Read32(hReg, BCHP_TVM_PAD_CNTL_3);

    lval &= ~(BCHP_TVM_PAD_CNTL_3_FORCE_HP_OUT_HIGH_MASK | 
                   BCHP_TVM_PAD_CNTL_3_FORCE_HP_OUT_LOW_MASK |
                       BCHP_TVM_PAD_CNTL_3_HOTPLUG_OUT_PAD_OEb_MASK);
    lval |= (BCHP_TVM_PAD_CNTL_3_EN_DVP_HR_HOTPLUG_IN_MASK | 
                BCHP_TVM_PAD_CNTL_3_EN_DVP_HR_HOTPLUG_OUT_MASK);

    BREG_Write32(hReg, BCHP_TVM_PAD_CNTL_3, lval);
#else
    BSTD_UNUSED(lval);
    BSTD_UNUSED(hReg);
#endif
}


void NEXUS_HdmiInputTvm_P_HotPlugDisable(void)
{
    uint32_t lval;
    BREG_Handle hReg = g_pCoreHandles->reg;

#if (BCHP_CHIP==35230)
    lval = BREG_Read32(hReg, BCHP_TVM_PAD_CTRL_4);

    lval &= ~(BCHP_TVM_PAD_CTRL_4_FORCE_HP_OUT_HIGH_MASK | 
                   BCHP_TVM_PAD_CTRL_4_FORCE_HP_OUT_LOW_MASK);

    BREG_Write32(hReg, BCHP_TVM_PAD_CTRL_4, lval);
#elif (BCHP_CHIP == 35125)
    lval = BREG_Read32(hReg, BCHP_TVM_PAD_CNTL_3);

    lval &= ~(BCHP_TVM_PAD_CNTL_3_FORCE_HP_OUT_HIGH_MASK | 
                   BCHP_TVM_PAD_CNTL_3_FORCE_HP_OUT_LOW_MASK);

    BREG_Write32(hReg, BCHP_TVM_PAD_CNTL_3, lval);
#else
    BSTD_UNUSED(lval);
    BSTD_UNUSED(hReg);
#endif
}



void NEXUS_HdmiInputTvm_P_EdidRam_Enable(void)
{
    uint32_t lval;
    BREG_Handle hReg = g_pCoreHandles->reg;

    /* program clock dividers */
    BREG_Write32(hReg, BCHP_TVM_CLOCK_DIVIDERS_5, 0x00010001);
    BREG_Write32(hReg, BCHP_TVM_CLOCK_DIVIDERS_6, 0x00010001);    
    BREG_Write32(hReg, BCHP_TVM_CLOCK_DIVIDERS_7, 0x00010001);

    lval = BREG_Read32(hReg, BCHP_TVM_EDID_ADDR_CNTRL1);
    lval &= ~(BCHP_TVM_EDID_ADDR_CNTRL1_TEN_BIT_ADDR_EN_MASK | 
                  BCHP_TVM_EDID_ADDR_CNTRL1_DEVICE_ADDR_MASK);
    BREG_Write32(hReg, BCHP_TVM_EDID_ADDR_CNTRL1, lval);

    lval = BREG_Read32(hReg, BCHP_TVM_EDID_MISC_CNTRL1);
    lval &= ~(BCHP_TVM_EDID_MISC_CNTRL1_SELF_TEST_EN_MASK);
    lval |= (1 << BCHP_TVM_EDID_MISC_CNTRL1_EDID_MODE_EN_SHIFT);
    lval |= (1 << BCHP_TVM_EDID_MISC_CNTRL1_INTERFACE_INPUT_EN_SHIFT);
    BREG_Write32(hReg, BCHP_TVM_EDID_MISC_CNTRL1, lval);
}


void NEXUS_HdmiInputTvm_P_EdidRam_LoadData(const uint8_t *dataBytes, uint16_t numEdidBytes)
{
    uint32_t i, lval;
    BREG_Handle hReg = g_pCoreHandles->reg;

    /* Put TVM_EDID_READRAM_WRITE_PORT1 into command mode for the 1st write.
        Should be set to 1 at the beginning of 1st transaction
    */
    lval = BREG_Read32(hReg, BCHP_TVM_EDID_DEBUG_REGISTER1);
    lval |= (1 << BCHP_TVM_EDID_DEBUG_REGISTER1_FIFO_EN_SHIFT);
    BREG_Write32(hReg, BCHP_TVM_EDID_DEBUG_REGISTER1, lval);

    /* TVM_EDID_READRAM_WRITE_PORT1 */
    /* In Command Mode:
        Bit 7:0 = The start address of the FIFO region to be filled.  Possible values are 0 to 255.
        Bit 15:8 = The end address of the FIFO region to be filled.  Possible values are 0 to 255.
        Note that END_ADDR must not be less than START_ADDR

        In Data Mode:
        Bit 7:0 = Byte to be stored at current write pointer.
        Bit 15:8 = Byte to be stored at current (write pointer +1)
        Bit 23:16 = Byte to be stored at current (write pointer + 2)
        Bit 31:24 = Byte to be stored at current (write pointer + 3)
    */
    BREG_Write32(hReg, BCHP_TVM_EDID_READRAM_WRITE_PORT1, 0x0000FF00);  /* for command mode */

    /* writing to ReadRam */
    for ( i=0; i < (numEdidBytes/4); i++ ) {
        lval =   *(dataBytes+(i*4));
        lval |= (*(dataBytes+1+(i*4))) << 8;
        lval |= (*(dataBytes+2+(i*4))) << 16;
        lval |= (*(dataBytes+3+(i*4))) << 24;
        
        BREG_Write32(hReg, BCHP_TVM_EDID_READRAM_WRITE_PORT1, lval);   /* for data mode */
        #if 0
        printf( "%08x ", lval );
        if ( (i +1)%4 == 0) printf( "\n" );
        #endif
    }

    lval = BREG_Read32(hReg, BCHP_TVM_EDID_MISC_CNTRL1);
    lval &= ~(BCHP_TVM_EDID_MISC_CNTRL1_EDID_MEM_RDY_FORCE_MASK);
    lval |= (1 << BCHP_TVM_EDID_MISC_CNTRL1_EDID_MEM_RDY_FORCE_SHIFT);  /* this must be done or the I2C will be unreadable by the HDMI source */
    BREG_Write32(hReg, BCHP_TVM_EDID_MISC_CNTRL1, lval);    
}



