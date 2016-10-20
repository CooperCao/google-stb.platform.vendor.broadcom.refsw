/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
* Module Description:
*
***************************************************************************/
#include "bhdm_mhl_host_priv.h"

BDBG_MODULE(BHDM_MHL_HOST);
BDBG_OBJECT_ID(HDMI_MHL_HOST);

static uint32_t BHDM_P_Mhl_Host_ReadScratchReg_isr
    ( BREG_Handle      hRegister,
      uint32_t         which )
{
    uint32_t ulData;
    uint32_t ulOffset = which * sizeof(uint32_t);

    BDBG_ASSERT(which < BHDM_P_MHL_HOST_NUM_SCRATCH_REGS);

    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_SW_SPARE0 + ulOffset);

    BDBG_MSG(("Reading scratch register 0x%x = 0x%x", BCHP_MPM_CPU_CTRL_SW_SPARE0 + ulOffset, ulData));

    return ulData;
}

static void BHDM_P_Mhl_Host_WriteScratchReg_isr
    ( BREG_Handle      hRegister,
      uint32_t         which,
      uint32_t         value )
{
    uint32_t ulOffset = which * sizeof(uint32_t);

    BDBG_ASSERT(which < BHDM_P_MHL_HOST_NUM_SCRATCH_REGS);

    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_SW_SPARE0 + ulOffset, value);
    BDBG_MSG(("Writing scratch register 0x%x = 0x%x", BCHP_MPM_CPU_CTRL_SW_SPARE0 + ulOffset, value));
}


/* Get a value from the scratch pad register */
uint8_t BHDM_P_Mhl_Host_GetField_isr
    ( BREG_Handle hRegister,
      uint16_t    offset,
      uint8_t     mask,
      uint8_t     lsb )
{
    uint32_t which = offset  >> 2;
    uint32_t bit_offset = (offset & 0x3) * 8;
    uint32_t reg = BHDM_P_Mhl_Host_ReadScratchReg_isr(hRegister, which);
    reg >>= bit_offset;
    return ((uint8_t)(reg >> lsb) & mask);
}

uint8_t BHDM_P_Mhl_Host_GetField
    ( BREG_Handle hRegister,
      uint16_t    offset,
      uint8_t     mask,
      uint8_t     lsb )
{
    uint8_t ucData;
    BKNI_EnterCriticalSection();
    ucData = BHDM_P_Mhl_Host_GetField_isr(hRegister, offset, mask, lsb);
    BKNI_LeaveCriticalSection();
    return (ucData);
}

/* Set a value in scratch pad register */
void BHDM_P_Mhl_Host_SetField_isr
    ( BREG_Handle hRegister,
      uint16_t    offset,
      uint8_t     mask,
      uint8_t     lsb,
      uint8_t     value )
{
    uint32_t which = offset  >> 2;
    uint32_t bit_offset = (offset & 0x3) * 8;
    uint32_t reg = BHDM_P_Mhl_Host_ReadScratchReg_isr(hRegister, which);
    uint8_t byte_reg = (reg >> bit_offset) & 0xFF;
    byte_reg &= ~(mask << lsb);
    byte_reg |= ((value & mask) << lsb);
    reg &= ~(0xFF << bit_offset);
    reg |= (byte_reg << bit_offset);
    BHDM_P_Mhl_Host_WriteScratchReg_isr(hRegister, which, reg);
}

void BHDM_P_Mhl_Host_SetField
    ( BREG_Handle hRegister,
      uint16_t    offset,
      uint8_t     mask,
      uint8_t     lsb,
      uint8_t     value )
{
    BKNI_EnterCriticalSection();
    BHDM_P_Mhl_Host_SetField_isr(hRegister, offset, mask, lsb, value);
    BKNI_LeaveCriticalSection();
}

/* This is the interrupt to notify the host
   that MPM is running */
void BHDM_P_Mhl_Host_S0ToS3Handover_isr
    ( BREG_Handle      hRegister )
{
    uint32_t ulData;

#if 0
    /* Mark host  as idle */
    BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, HOST_STATE, BHDM_P_Mhl_HostCpuState_eIdle);
    BDBG_MSG(("CPU is now idle"));
#endif

    /* Enable CPU clock */
    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL);
    ulData &= ~BCHP_MPM_CPU_CTRL_CLOCK_CTRL_DISABLE_CPU_CLOCK_MASK;
    ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_CLOCK_CTRL, DISABLE_CPU_CLOCK, 0);
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL, ulData);

#if BHDM_MHL_ENABLE_DEBUG
    /* Enable UART interrupts */
    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL);
    ulData &= ~BCHP_MPM_CPU_CTRL_CLOCK_CTRL_DISABLE_UART_CLOCK_MASK;
    ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_CLOCK_CTRL, DISABLE_UART_CLOCK, 0);
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL, ulData);
#endif

    /* Reboot MPM CPU */
    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL);
    ulData &= ~BCHP_MPM_CPU_CTRL_RESET_CTRL_CPU_RESET_ONE_SHOT_MASK;
    ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_RESET_CTRL, CPU_RESET_ONE_SHOT, 1);
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_RESET_CTRL, ulData);

    /* Mark host  as idle */
    BHDM_P_MHL_MAILBOX_SET_FIELD_ISR(hRegister, HOST_STATE, BHDM_P_Mhl_HostCpuState_eStopping);
    BDBG_MSG(("Host is now stopping"));
}

/* Call this when TX_READY interrupt is triggered */
void BHDM_P_Mhl_Host_S3ToS0Handover_isr
    ( BREG_Handle      hRegister )
{
    BREG_Write32(hRegister, BCHP_HDMI_TX_PHY_POWERDOWN_CTL,
        BCHP_FIELD_DATA(HDMI_TX_PHY_POWERDOWN_CTL, TX_PWRDN_SEL, 0));

#if BHDM_MHL_ENABLE_DEBUG
    /* Disable UART interrupts */
    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL);
    ulData &= ~BCHP_MPM_CPU_CTRL_CLOCK_CTRL_DISABLE_UART_CLOCK_MASK;
    ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_CLOCK_CTRL, DISABLE_UART_CLOCK, 1);
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL, ulData);
#endif

#if 0 /* TODO: Handle if firmware update is needed */
    /* Disable SPI if FW update is not needed */
    if (!bFwUpdate)
    {
        ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL);
        ulData &= ~BCHP_MPM_CPU_CTRL_CLOCK_CTRL_DISABLE_MPM_SPI_CLOCK_MASK;
        ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_CLOCK_CTRL, DISABLE_MPM_SPI_CLOCK, 1);
        BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_CLOCK_CTRL, ulData);
    }
#endif

}

/* Start the power management state machine and DPHY */
void BHDM_P_Mhl_Host_StartPmsm_isr
    ( BREG_Handle      hRegister )
{
    uint32_t ulData;
    BDBG_MSG(("Starting AON PMSM"));

    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_PM_CTRL);
    ulData &= ~(BCHP_MPM_CPU_CTRL_PM_CTRL_MPM_WAKEUP_SYSTEM_MASK |
                BCHP_MPM_CPU_CTRL_PM_CTRL_RELEASE_DPHY_STANDBY_MASK);
    ulData |= (BCHP_FIELD_DATA(MPM_CPU_CTRL_PM_CTRL, MPM_WAKEUP_SYSTEM, 1) |
                BCHP_FIELD_DATA(MPM_CPU_CTRL_PM_CTRL, RELEASE_DPHY_STANDBY, 1));
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_PM_CTRL, ulData);

    /* This field requires oneshot */
    ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_PM_CTRL, RELEASE_DPHY_STANDBY, 0);
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_PM_CTRL, ulData);
}

/* Enable the TMDS path of MHL */
void BHDM_P_Mhl_Host_EnableMhlTx_isr
    ( BREG_Handle      hRegister,
      bool             enable )
{
    uint32_t ulData;

    BDBG_MSG(("Turning %s MHL TMDS", (enable)? "on" : "off"));

    ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_MISC_CTRL);
    ulData &= ~(BCHP_MPM_CPU_CTRL_MISC_CTRL_MHL_TX_ON_MASK);
    ulData |= BCHP_FIELD_DATA(MPM_CPU_CTRL_MISC_CTRL, MHL_TX_ON, (enable) ? 1 : 0);
    BREG_Write32(hRegister, BCHP_MPM_CPU_CTRL_MISC_CTRL, ulData);
}

/* Read the interrupt status registers */
bool BHDM_P_Mhl_Host_GetIntrStatus
    ( BREG_Handle             hRegister,
      BHDM_P_Mhl_MpmHostIntr *pIntrStat )
{
    pIntrStat->ulStatus = BREG_Read32(hRegister, BCHP_MPM_HOST_L2_CPU_STATUS);

    /* This is just a short cut to see if there is any interrupts at all */
    return (pIntrStat->ulStatus) ? true : false;
}

/* Clear all interrupts */
void BHDM_P_Mhl_Host_ClearAllInt_isr
    ( BREG_Handle          hRegister )
{
    uint32_t ulClearMask = 0xffffffff;

    BREG_Write32(hRegister, BCHP_MPM_HOST_L2_CPU_CLEAR, ulClearMask);
    BDBG_MSG(("Clearing all MPM HOST L2 interrupts."));
}

void BHDM_P_Mhl_Host_ClearInt_isr
    ( BREG_Handle              hRegister,
      BHDM_P_Mhl_MpmHostIntr  *pIntStat )
{
    /* Define these masks if we want to mask some intrs
       in the log. The interrupts will always be cleared.
       A mask of 0x0 means all intrs will be logged.
       An example of intr0 mask might be:

       CBUS_INTR2_0_CPU_STATUS_INITIATOR_PKT_SET |
       CBUS_INTR2_0_CPU_STATUS_FOLLOWER_PKT_SET

       which means initiator packet/follower packet interrupts
       are not logged but will still be cleared.
    */

#ifndef BHDM_P_MHL_MPM_HOST_INTR0_LOG_IGNORE_MASK
#define BHDM_P_MHL_MPM_HOST_INTR0_LOG_IGNORE_MASK 0x0
#endif


#if BHDM_MHL_ENABLE_DEBUG
    uint32_t ulMpmHostIntrIgnoreMask;

    /* If only the following interrupts are fired,
       we don't print the log out, set both of these
       to zero if we want to log all interrupts fired */
    ulMpmHostIntrIgnoreMask = BHDM_P_MHL_MPM_HOST_INTR0_LOG_IGNORE_MASK;


    if(pIntStat->ulStatus & ~ulMpmHostIntrIgnoreMask)
    {
        uint32_t ulStatus;
        /* Note that we just print the current intr values out, not
           updating the values stored in pIntStat */
        ulStatus = BREG_Read32(hRegister, BCHP_MPM_HOST_L2_CPU_STATUS);

        BDBG_MSG(("Current MPM_HOST_INTR:0x%x Clearing MPM_HOST_INTR:0x%x", ulStatus, pIntStat->ulStatus));
    }
#endif

    if(pIntStat->ulStatus)
    {
        BREG_Write32(hRegister, BCHP_MPM_HOST_L2_CPU_CLEAR, pIntStat->ulStatus);
    }
}

#if BHDM_MHL_CTS
void BHDM_P_Mhl_Host_MailboxInit
    ( BREG_Handle      hRegister,
      uint8_t          rev,
      uint8_t         *pucDcap )
{
    uint8_t src_link_mode;
    uint8_t src_clk_mode;

    src_link_mode = BHDM_P_MHL_HOST_GET_FIELD(hRegister, SRC_LINK_MODE_2);
    BDBG_MSG(("Initialising mailbox from FCB SRC_LINK_MODE_2 from FCB is %d", src_link_mode));

    BHDM_P_Mhl_Mailbox_Init(hRegister, rev, pucDcap, src_link_mode);

    /* Sanity check on source clock mode */
    src_clk_mode = BHDM_P_MHL_MAILBOX_GET_FIELD(hRegister, SRC_CLK_MODE_1);

    if (src_clk_mode != BHDM_P_Mhl_ClkMode_ePacked && src_clk_mode != BHDM_P_Mhl_ClkMode_e24bit)
    {
        BDBG_WRN(("Invalid source clk_mode in FCB, default to 24-bit"));
        BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, SRC_CLK_MODE_1, BHDM_P_Mhl_ClkMode_e24bit);
    }

    BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, SINK_DCAP_RDY,
                   BHDM_P_MHL_HOST_GET_FIELD(hRegister, SINK_DCAP_RDY));
    BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, HPD,
                   BHDM_P_MHL_HOST_GET_FIELD(hRegister, HPD));
    BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, SRC_MSC_ERRORCODE,
                   BHDM_P_MHL_HOST_GET_FIELD(hRegister, SRC_MSC_ERRORCODE));
    BHDM_P_MHL_MAILBOX_SET_FIELD(hRegister, SRC_DDC_ERRORCODE,
                   BHDM_P_MHL_HOST_GET_FIELD(hRegister, SRC_DDC_ERRORCODE));
}
#endif
