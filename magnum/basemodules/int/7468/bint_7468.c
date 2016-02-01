/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/

#include "bstd.h"
#include "bint_7468.h"

/* Include interrupt definitions from RDB */
#include "bchp_hif_cpu_intr1.h"

/* Standard L2 stuff */
#include "bchp_aio_inth.h"
#include "bchp_aud_dsp_inth0.h"
#include "bchp_avd_intr2_0.h"

#include "bchp_bvnb_intr2.h"
#include "bchp_bvnf_intr2_0.h"
#include "bchp_bvnf_intr2_1.h"
#include "bchp_bvnf_intr2_3.h"
#include "bchp_bvnf_intr2_4.h"
#include "bchp_bvnf_intr2_5.h"
#include "bchp_bvnf_intr2_6.h"
#include "bchp_hdmi_intr2.h"
#include "bchp_mcif_intr2.h"
#include "bchp_sun_l2.h"
#include "bchp_usb_intr2.h"
#include "bchp_video_enc_intr2.h"
#include "bchp_bsp_control_intr2.h"
#include "bchp_m2mc_wrap_l2.h"
#include "bchp_memc_l2_0.h"

#include "bchp_pm_l2.h"

/* XPT */
#include "bchp_xpt_fe.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_dpcr2.h"
#include "bchp_xpt_dpcr3.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_pb2.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_msg.h"
#include "bchp_xpt_pcroffset.h"

/* UARTs, keypad, I2C */
#include "bchp_irq0.h"

/* Smartcard interrupts. */
#include "bchp_scirq0.h"

/* Timer */
#include "bchp_timer.h"


#ifdef INCLUDE_SM
	#include "bchp_sm_l2.h"
#endif /* INCLUDE_SM */

BDBG_MODULE(interruptinterface_7468);

#define BINT_P_STD_STATUS       0x00
#define BINT_P_STD_SET          0x04
#define BINT_P_STD_CLEAR        0x08
#define BINT_P_STD_MASK_STATUS  0x0C
#define BINT_P_STD_MASK_SET     0x10
#define BINT_P_STD_MASK_CLEAR   0x14

#define BINT_P_IRQ0_CASES \
    case BCHP_IRQ0_IRQEN: 

#define BINT_P_IRQ0_ENABLE      0
#define BINT_P_IRQ0_STATUS      4


#define BINT_P_XPT_STATUS           0x00
#define BINT_P_XPT_ENABLE           0x04

#define BINT_P_XPT_STATUS_CASES \
    case BCHP_XPT_BUS_IF_INTR_STATUS_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS2_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS3_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS4_REG: \
    case BCHP_XPT_BUS_IF_INTR_STATUS5_REG: \
    case BCHP_XPT_PB0_INTR: \
    case BCHP_XPT_PB1_INTR: \
    case BCHP_XPT_PB2_INTR: \
    case BCHP_XPT_DPCR0_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR1_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR2_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR3_INTR_STATUS_REG: 

#define BINT_P_PCROFFSET_CASES \
    case BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS: 

#define BINT_P_PCROFFSET_STATUS           0x00
#define BINT_P_PCROFFSET_ENABLE           0x04


/* There is no constant address mapping from RAVE status to RAVE enable registers. */
#define BINT_P_RAVE_STATUS          0x00

#define BINT_P_XPT_RAVE_CASES \
    case BCHP_XPT_RAVE_INT_CX0: \
    case BCHP_XPT_RAVE_INT_CX1: \
    case BCHP_XPT_RAVE_INT_CX2: \
    case BCHP_XPT_RAVE_INT_CX3: \
    case BCHP_XPT_RAVE_INT_CX4: \
    case BCHP_XPT_RAVE_INT_CX5: \
    case BCHP_XPT_RAVE_INT_CX6: \
    case BCHP_XPT_RAVE_INT_CX7: \
    case BCHP_XPT_RAVE_INT_CX8: \
    case BCHP_XPT_RAVE_INT_CX9: \
    case BCHP_XPT_RAVE_INT_CX10: \
    case BCHP_XPT_RAVE_INT_CX11: \
    case BCHP_XPT_RAVE_INT_CX12: \
    case BCHP_XPT_RAVE_INT_CX13: \
    case BCHP_XPT_RAVE_INT_CX14: \
    case BCHP_XPT_RAVE_INT_CX15: \
    case BCHP_XPT_RAVE_INT_CX16: \
    case BCHP_XPT_RAVE_INT_CX17: \
    case BCHP_XPT_RAVE_INT_CX18: \
    case BCHP_XPT_RAVE_INT_CX19: \
    case BCHP_XPT_RAVE_INT_CX20: \
    case BCHP_XPT_RAVE_INT_CX21: \
    case BCHP_XPT_RAVE_INT_CX22: \
    case BCHP_XPT_RAVE_INT_CX23:

#define BINT_P_XPT_BUF_STATUS       0x00
#define BINT_P_XPT_BUF_ENABLE       0x10

#define BINT_P_XPT_BUF_CASES \
    case BCHP_XPT_MSG_BUF_OVFL_INTR_00_31: \
    case BCHP_XPT_MSG_BUF_OVFL_INTR_32_63: \
    case BCHP_XPT_MSG_BUF_OVFL_INTR_64_95: \
    case BCHP_XPT_MSG_BUF_OVFL_INTR_96_127: \
    case BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31: \
    case BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63: \
    case BCHP_XPT_MSG_BUF_DAT_RDY_INTR_64_95: \
    case BCHP_XPT_MSG_BUF_DAT_RDY_INTR_96_127: 

#define BINT_P_XPT_MSG_ERR_STATUS   ( 0x00 )
#define BINT_P_XPT_MSG_ERR_ENABLE   ( 0x04 )
#define BINT_P_XPT_MSG_ERR_CASES \
    case BCHP_XPT_MSG_DAT_ERR_INTR:

/* BINT_P_UPGSC_ENABLE was defined as -4 for BCHP_SCIRQ0_SCIRQSTAT.
 * Since we are using BCHP_SCIRQ0_SCIRQEN, it is not needed but
 * to minimize the change, it is kept and set to 0 
 */ 
#define BINT_P_UPGSC_ENABLE (0)

#define BINT_P_UPGSC_CASES \
    case BCHP_SCIRQ0_SCIRQEN:

#define BINT_P_TIMER_STATUS     0x00
#define BINT_P_TIMER_MASK       0x04

#define BINT_P_TIMER_CASES \
    case BCHP_TIMER_TIMER_IS:

#if 0
#define BINT_P_UHF_STATUS       0x04
#define BINT_P_UHF_MASK         0x00
    
#define BINT_P_UHF_CASES \
    case BCHP_UHFR_1_IMSK:
#endif

#define BINT_P_STAT_TIMER_TICKS_PER_USEC 27

#if 0
#define BINT_P_AFEC_CASES \
        case BCHP_AFEC_uP_IRQ_STAT:

#define BINT_P_AFEC_STATUS    0x00
#define BINT_P_AFEC_MASK      0x18
#define BINT_P_AFEC_IRQ_SET   0x10
#define BINT_P_AFEC_IRQ_RESET 0x20
#endif

static void BINT_P_7468_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7468_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7468_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static uint32_t BINT_P_7468_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );
static uint32_t GetRaveIntEnableOffset( uint32_t BaseAddr );

static const BINT_P_IntMap bint_7468[] =
{
    BINT_MAP(0, XPT_RAV, "_CX0", XPT_RAVE_INT_CX0, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX1", XPT_RAVE_INT_CX1, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX2", XPT_RAVE_INT_CX2, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX3", XPT_RAVE_INT_CX3, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX4", XPT_RAVE_INT_CX4, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX5", XPT_RAVE_INT_CX5, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX6", XPT_RAVE_INT_CX6, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX7", XPT_RAVE_INT_CX7, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX8", XPT_RAVE_INT_CX8, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX9", XPT_RAVE_INT_CX9, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX10", XPT_RAVE_INT_CX10, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX11", XPT_RAVE_INT_CX11, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX12", XPT_RAVE_INT_CX12, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX13", XPT_RAVE_INT_CX13, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX14", XPT_RAVE_INT_CX14, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX15", XPT_RAVE_INT_CX15, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX16", XPT_RAVE_INT_CX16, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX17", XPT_RAVE_INT_CX17, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX18", XPT_RAVE_INT_CX18, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX19", XPT_RAVE_INT_CX19, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX20", XPT_RAVE_INT_CX20, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX21", XPT_RAVE_INT_CX21, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX22", XPT_RAVE_INT_CX22, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX23", XPT_RAVE_INT_CX23, WEAK_MASK, ALL, 0),

    BINT_MAP_STD(0, AVD0, AVD_INTR2_0_CPU),
    BINT_MAP_STD(0, BVNF_6, BVNF_INTR2_6_R5F),
    BINT_MAP_STD(0, BVNF_5, BVNF_INTR2_5_R5F),

    /* BINT_MAP(0, UHF1, "", UHFR_1_IMSK, REGULAR, ALL, 0), */
    BINT_MAP(0, SUN, "", SUN_L2_CPU_STATUS, STANDARD, MASK, 0xFFF0003E),
    BINT_MAP(0, UPG_SC, "", SCIRQ0_SCIRQEN, REGULAR, ALL, 0),
    /*  spi_irqen */
    /* BINT_MAP(0, UPG_SPI, "", IRQ0_IRQEN, REGULAR, MASK, 0xFFEFFFFF), */
    /* iica_irqen,iicb_irqen,iicc_irqen,iicd_irqen */
    BINT_MAP(0, UPG_BSC, "", IRQ0_IRQEN, REGULAR, MASK, 0xF0FFFFFF),
    /* kbd1_irqen,ldk_irqen,irb_irqen,ua_irqen,kbd2_irqen,gio_irqen,icap_irqen,kbd3_irqen */
    BINT_MAP(0, UPG, "", IRQ0_IRQEN, REGULAR, MASK, 0xFFFFFC00),
    BINT_MAP(0, UPG_TMR, "", TIMER_TIMER_IS, REGULAR, ALL, 0),
    /* bicap_fifo_of, bicap_fifo_lvl, bicap_fifo_inact, bicap_timeout3, bicap_timeout2, bicap_timeout1, bicap_timeout0 */
    BINT_MAP(0, MC, "" , MCIF_INTR2_CPU_STATUS, STANDARD, MASK, 0xFFFC07FF),
    
    /* main memory controller block has become independent from graphics */
    BINT_MAP_STD(0, MEMC_0, MEMC_L2_0_R5F),
    BINT_MAP_STD(0, BVNF_1, BVNF_INTR2_1_R5F),
    BINT_MAP_STD(0, BVNF_0, BVNF_INTR2_0_R5F),
    BINT_MAP_STD(0, BVNB, BVNB_INTR2_CPU),
    BINT_MAP_STD(0, VEC, VIDEO_ENC_INTR2_CPU),
    BINT_MAP_STD(0, RPTD, AUD_DSP_INTH0_R5F),
    BINT_MAP_STD(0, HDMI, HDMI_INTR2_CPU),
    BINT_MAP_STD(0, M2MC, M2MC_WRAP_L2_CPU),
    /* BINT_MAP_STD(0, M2MC, MEM_DMA_0), */
	/* GR_BRIDGE_INTR,M2MC_0_INTR,MEM_DMA_0_INTR */
    /* BINT_MAP_STD(0, GFX, GFX_L2_CPU), */
    BINT_MAP_STD(0, AIO, AIO_INTH_R5F),
    BINT_MAP_STD(0, BSP, BSP_CONTROL_INTR2_CPU),
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_ICAM_CPU_INTR_SHIFT,            BCHP_XPT_BUS_IF_INTR_STATUS3_REG,      0,                   "XPT_ICAM"},
    BINT_MAP(0, XPT_MSG, "_0", XPT_MSG_BUF_DAT_RDY_INTR_00_31,       REGULAR, NONE, 0),
    BINT_MAP(0, XPT_MSG, "_1", XPT_MSG_BUF_DAT_RDY_INTR_32_63,       REGULAR, NONE, 0),
    BINT_MAP(0, XPT_MSG, "_2", XPT_MSG_BUF_DAT_RDY_INTR_64_95,       REGULAR, NONE, 0),
    BINT_MAP(0, XPT_MSG, "_3", XPT_MSG_BUF_DAT_RDY_INTR_96_127,      REGULAR, NONE, 0),
    BINT_MAP(0, XPT_OVFL, "_0", XPT_MSG_BUF_OVFL_INTR_00_31,  REGULAR, NONE, 0),
    BINT_MAP(0, XPT_OVFL, "_1", XPT_MSG_BUF_OVFL_INTR_32_63,  REGULAR, NONE, 0),
    BINT_MAP(0, XPT_OVFL, "_2", XPT_MSG_BUF_OVFL_INTR_64_95,  REGULAR, NONE, 0),
    BINT_MAP(0, XPT_OVFL, "_3", XPT_MSG_BUF_OVFL_INTR_96_127,  REGULAR, NONE,0),
    BINT_MAP(0, XPT_STATUS, "" , XPT_BUS_IF_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(0, XPT_STATUS, "2", XPT_BUS_IF_INTR_STATUS2_REG, REGULAR, ALL, 0),
    /*BINT_MAP(0, XPT_STATUS, "3", XPT_BUS_IF_INTR_STATUS3_REG, REGULAR, ALL, 0), */
    BINT_MAP(0, XPT_STATUS, "4", XPT_BUS_IF_INTR_STATUS4_REG, REGULAR, ALL, 0),
    BINT_MAP(0, XPT_STATUS, "5", XPT_BUS_IF_INTR_STATUS5_REG, REGULAR, ALL, 0),
    BINT_MAP(0, XPT_STATUS, "_PB0", XPT_PB0_INTR, REGULAR, ALL, 0),
    BINT_MAP(0, XPT_STATUS, "_PB1", XPT_PB1_INTR, REGULAR, ALL, 0),
    BINT_MAP(0, XPT_STATUS, "_PB2", XPT_PB2_INTR, REGULAR, ALL, 0),

    BINT_MAP(1, XPT_MSG_STAT, "", XPT_MSG_DAT_ERR_INTR,       REGULAR, ALL, 0),
    BINT_MAP(1, XPT_FE, "_STATUS", XPT_FE_INTR_STATUS_REG, STANDARD, ALL, 0),
    BINT_MAP(1, XPT_PCR, "0", XPT_DPCR0_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "1", XPT_DPCR1_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "2", XPT_DPCR2_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "3", XPT_DPCR3_INTR_STATUS_REG, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "OFF0", XPT_PCROFFSET_INTERRUPT0_STATUS, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "OFF1", XPT_PCROFFSET_INTERRUPT1_STATUS, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "OFF2", XPT_PCROFFSET_INTERRUPT2_STATUS, REGULAR, ALL, 0),
    BINT_MAP(1, XPT_PCR, "OFF3", XPT_PCROFFSET_INTERRUPT3_STATUS, REGULAR, ALL, 0),

    /* { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_HIF_SPI_CPU_INTR_SHIFT+ 64,  		BCHP_PM_L2_CPU_STATUS,  		0,                   "PM"}, */

    { -1, 0, 0, NULL} 
};

static const BINT_Settings bint_7468Settings =
{
    NULL,
    BINT_P_7468_ClearInt,
    BINT_P_7468_SetMask,
    BINT_P_7468_ClearMask,
    NULL,
    BINT_P_7468_ReadStatus, 
    bint_7468,
    "7468"
};

static void BINT_P_7468_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    BDBG_MSG(("ClearInt %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
#if 0
        BINT_P_AFEC_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_AFEC_IRQ_RESET, 1ul<<shift);
            break;
#endif
        BINT_P_XPT_STATUS_CASES 
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_STATUS, ~(1ul<<shift));
            break;
        BINT_P_XPT_RAVE_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_RAVE_STATUS, (1ul<<shift));
            break;
        BINT_P_XPT_BUF_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_BUF_STATUS, ~(1ul<<shift));
            break;
        BINT_P_TIMER_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_STATUS, 1ul<<shift);
            break;
#if 0
        BINT_P_UHF_CASES
            /* Interrupt is cleared twice for UHF interrupt. 
               Richa needs to look into this for real solution.
               BREG_Write32( regHandle, baseAddr + BINT_P_UHF_STATUS, 1ul<<shift); */
            break;
#endif
        BINT_P_UPGSC_CASES
        BINT_P_IRQ0_CASES
            /* Has to cleared at the source */
            break;
        BINT_P_XPT_MSG_ERR_CASES
            /* Write 0 to clear the int bit. Writing 1's are ingored. */
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_MSG_ERR_STATUS, ~( 1ul << shift ) );
            break;
        BINT_P_PCROFFSET_CASES
            /* Write 0 to clear the int bit. Writing 1's are ingored. */
            BREG_Write32( regHandle, baseAddr + BINT_P_PCROFFSET_STATUS, ~( 1ul << shift ) );
            break;
        default:
            /* Other types of interrupts do not support clearing of interrupts (condition must be cleared) */
            break;
    }
}

static void BINT_P_7468_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;
    
    uint32_t RaveEnReg = 0;

    BDBG_MSG(("SetMask %#x:%d", baseAddr, shift));

    switch( baseAddr )
    {
#if 0
    BINT_P_AFEC_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_AFEC_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_AFEC_MASK, intEnable);
        break;
#endif
    BINT_P_XPT_STATUS_CASES 
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_ENABLE, intEnable);
        break;
    BINT_P_XPT_RAVE_CASES
        RaveEnReg = GetRaveIntEnableOffset( baseAddr );
        intEnable = BREG_Read32( regHandle, RaveEnReg );
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, RaveEnReg, intEnable);
        break;
    BINT_P_XPT_BUF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE, intEnable);
        break;
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable);
        break;
#if 0
    BINT_P_UHF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UHF_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_UHF_MASK, intEnable);
        break;
#endif
    BINT_P_IRQ0_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE, intEnable);
        break;
    BINT_P_UPGSC_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE, intEnable );
        break;

    BINT_P_XPT_MSG_ERR_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_MSG_ERR_ENABLE );
        intEnable &= ~( 1ul << shift );
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_MSG_ERR_ENABLE, intEnable);
        break;

    BINT_P_PCROFFSET_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE );
        intEnable &= ~( 1ul << shift );
        BREG_Write32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE, intEnable);
        break;

    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}

static void BINT_P_7468_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;
    
    uint32_t RaveEnReg = 0;

    BDBG_MSG(("ClearMask %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
#if 0
    BINT_P_AFEC_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_AFEC_MASK);
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_AFEC_MASK, intEnable);
        break;
#endif
    BINT_P_XPT_STATUS_CASES 
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_ENABLE);
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_ENABLE, intEnable);
        break;
    BINT_P_XPT_RAVE_CASES
        RaveEnReg = GetRaveIntEnableOffset( baseAddr );
        intEnable = BREG_Read32( regHandle, RaveEnReg );
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, RaveEnReg, intEnable);
        break;
    BINT_P_XPT_BUF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE);
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_BUF_ENABLE, intEnable);
        break;
#if 0
    BINT_P_UHF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UHF_MASK);
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_UHF_MASK, intEnable);
        break;
#endif
    BINT_P_IRQ0_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE);
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE, intEnable );
        break;
    BINT_P_UPGSC_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
        intEnable |= 1ul<<shift;
        BREG_Write32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE, intEnable );
        break;
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK );
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable );
        break;

    BINT_P_XPT_MSG_ERR_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_XPT_MSG_ERR_ENABLE );
        intEnable |= ( 1ul << shift );
        BREG_Write32( regHandle, baseAddr + BINT_P_XPT_MSG_ERR_ENABLE, intEnable);
        break;

    BINT_P_PCROFFSET_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE );
        intEnable |= ( 1ul << shift );
        BREG_Write32( regHandle, baseAddr + BINT_P_PCROFFSET_ENABLE, intEnable);
        break;

    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }
}

static uint32_t BINT_P_7468_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
    BDBG_MSG(("ReadStatus %#x", baseAddr));
    switch( baseAddr )
    {
#if 0
    BINT_P_AFEC_CASES
        return BREG_Read32(regHandle, baseAddr + BINT_P_STD_STATUS);
#endif
    BINT_P_XPT_STATUS_CASES 
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_STATUS );
    BINT_P_XPT_RAVE_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_RAVE_STATUS );
    BINT_P_XPT_BUF_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_BUF_STATUS );
    BINT_P_TIMER_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_STATUS );
#if 0
    BINT_P_UHF_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_UHF_STATUS );
#endif
    BINT_P_IRQ0_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_STATUS );
    BINT_P_UPGSC_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
    BINT_P_XPT_MSG_ERR_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_MSG_ERR_STATUS );
    BINT_P_PCROFFSET_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_STATUS );
    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        return 0;
    }
}

#if 0
static void verify_map(const BINT_P_IntMap *m1, const BINT_P_IntMap *m2)
{
    unsigned i;
    for(i=0;;i++) {
        int m1l1s = m1[i].L1Shift & ~(BINT_P_MAP_MISC_WEAK_MASK | BINT_P_MAP_MISC_STANDARD);
        int m2l1s = m1[i].L1Shift & ~(BINT_P_MAP_MISC_WEAK_MASK | BINT_P_MAP_MISC_STANDARD);
        if(m1l1s != m2l1s || m1[i].L2RegOffset != m2[i].L2RegOffset || m1[i].L2InvalidMask != m2[i].L2InvalidMask) {
            BDBG_WRN(("found mismatch at %u: %d:%d %u:%u %#x:%#x %s:%s", i, m1l1s, m2l1s, m1[i].L2RegOffset, m2[i].L2RegOffset, m1[i].L2InvalidMask, m2[i].L2InvalidMask, m1[i].L2Name, m2[i].L2Name));
        }
        if(m1[i].L1Shift == -1 || m1[i].L1Shift == -1) {
            break;
        }
    }
    return;
}
#endif

const BINT_Settings *BINT_7468_GetSettings( void )
{
    return &bint_7468Settings;
}


static uint32_t GetRaveIntEnableOffset( 
    uint32_t BaseAddr 
    )
{
    uint32_t EnableAddr = 0;

    switch( BaseAddr )
    {
        case BCHP_XPT_RAVE_INT_CX0: EnableAddr =   BCHP_XPT_RAVE_CX0_AV_INTERRUPT_ENABLES; break;
        case BCHP_XPT_RAVE_INT_CX1: EnableAddr =   BCHP_XPT_RAVE_CX1_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX2: EnableAddr =   BCHP_XPT_RAVE_CX2_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX3: EnableAddr =   BCHP_XPT_RAVE_CX3_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX4: EnableAddr =   BCHP_XPT_RAVE_CX4_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX5: EnableAddr =   BCHP_XPT_RAVE_CX5_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX6: EnableAddr =   BCHP_XPT_RAVE_CX6_AV_INTERRUPT_ENABLES; break;
        case BCHP_XPT_RAVE_INT_CX7: EnableAddr =   BCHP_XPT_RAVE_CX7_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX8: EnableAddr =   BCHP_XPT_RAVE_CX8_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX9: EnableAddr =   BCHP_XPT_RAVE_CX9_AV_INTERRUPT_ENABLES; break;   
        case BCHP_XPT_RAVE_INT_CX10: EnableAddr =  BCHP_XPT_RAVE_CX10_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX11: EnableAddr =  BCHP_XPT_RAVE_CX11_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX12: EnableAddr =  BCHP_XPT_RAVE_CX12_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX13: EnableAddr =  BCHP_XPT_RAVE_CX13_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX14: EnableAddr =  BCHP_XPT_RAVE_CX14_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX15: EnableAddr =  BCHP_XPT_RAVE_CX15_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX16: EnableAddr =  BCHP_XPT_RAVE_CX16_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX17: EnableAddr =  BCHP_XPT_RAVE_CX17_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX18: EnableAddr =  BCHP_XPT_RAVE_CX18_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX19: EnableAddr =  BCHP_XPT_RAVE_CX19_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX20: EnableAddr =  BCHP_XPT_RAVE_CX20_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX21: EnableAddr =  BCHP_XPT_RAVE_CX21_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX22: EnableAddr =  BCHP_XPT_RAVE_CX22_AV_INTERRUPT_ENABLES; break;  
        case BCHP_XPT_RAVE_INT_CX23: EnableAddr =  BCHP_XPT_RAVE_CX23_AV_INTERRUPT_ENABLES; break;  

        default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        break;
    }

    return EnableAddr;
}


/* End of file */

