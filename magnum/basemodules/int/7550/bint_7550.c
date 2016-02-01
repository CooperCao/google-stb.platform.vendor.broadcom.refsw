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
#include "bint_7550.h"

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
#include "bchp_hdmi_intr2.h"
#include "bchp_mcif_intr2.h"
#include "bchp_sun_l2.h"
#include "bchp_usb_intr2.h"
#include "bchp_usb_ehci.h"
#include "bchp_usb_ohci.h"
#include "bchp_video_enc_intr2.h"
#include "bchp_memc_l2_0.h"
#include "bchp_thd_intr2.h"
#if BCHP_VER >= BCHP_VER_B0
#include "bchp_m2mc_top_l2.h"
#include "bchp_sharf_intr2.h"
#endif

/* XPT */
#include "bchp_xpt_fe.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_pcroffset.h"

/* UARTs, keypad, I2C */
#include "bchp_irq0.h"

/* Smartcard interrupts. */
#include "bchp_scirq0.h"

/* Timer */
#include "bchp_timer.h"

/* Tuner */
#include "bchp_ds_tuner.h"
#include "bchp_ds.h"

BDBG_MODULE(interruptinterface_7550);

#define BINT_P_STD_STATUS       0x00
#define BINT_P_STD_SET          0x04
#define BINT_P_STD_CLEAR        0x08
#define BINT_P_STD_MASK_STATUS  0x0C
#define BINT_P_STD_MASK_SET     0x10
#define BINT_P_STD_MASK_CLEAR   0x14

#define BINT_P_STANDARD_7550_AX_L2_CASES \
    case BCHP_AIO_INTH_R5F_STATUS: \
    case BCHP_AUD_DSP_INTH0_R5F_STATUS: \
    case BCHP_AVD_INTR2_0_CPU_STATUS: \
    case BCHP_BVNB_INTR2_CPU_STATUS: \
    case BCHP_BVNF_INTR2_0_R5F_STATUS: \
    case BCHP_BVNF_INTR2_1_R5F_STATUS: \
    case BCHP_BVNF_INTR2_3_AVD0_STATUS: \
    case BCHP_BVNF_INTR2_4_AVD0_STATUS: \
    case BCHP_BVNF_INTR2_5_R5F_STATUS: \
    case BCHP_HDMI_INTR2_CPU_STATUS: \
    case BCHP_MCIF_INTR2_CPU_STATUS: \
    case BCHP_VIDEO_ENC_INTR2_CPU_STATUS: \
    case BCHP_SUN_L2_CPU_STATUS: \
    case BCHP_THD_INTR2_CPU_STATUS: \
    case BCHP_MEMC_L2_0_R5F_STATUS: \
    case BCHP_DS_TUNER_IRQSTS: \
    case BCHP_DS_IRQSTS1:\
    case BCHP_DS_IRQSTS2:

#if BCHP_VER >= BCHP_VER_B0
#define BINT_P_STANDARD_L2_CASES BINT_P_STANDARD_7550_AX_L2_CASES  \
    case BCHP_M2MC_TOP_L2_CPU_STATUS: \
    case BCHP_SHARF_INTR2_CPU_STATUS:
#else        
#define BINT_P_STANDARD_L2_CASES BINT_P_STANDARD_7550_AX_L2_CASES
#endif       

#define BINT_P_PCROFFSET_CASES \
    case BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS: \
    case BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS:

#define BINT_P_PCROFFSET_STATUS           0x00
#define BINT_P_PCROFFSET_ENABLE           0x04

#define BINT_P_IRQ0_CASES \
    case BCHP_IRQ0_IRQEN: \
    case BCHP_IRQ0_GPIO_IRQEN: \
    case BCHP_IRQ0_BSC_IRQEN:\
    case BCHP_IRQ0_SPI_IRQEN:

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
    case BCHP_XPT_FE_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR0_INTR_STATUS_REG: \
    case BCHP_XPT_PB0_INTR: \
    case BCHP_XPT_PB1_INTR: \
    case BCHP_XPT_BUS_IF_MISC_CTRL0:

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

#define BINT_P_STAT_TIMER_TICKS_PER_USEC 27

static void BINT_P_7550_SetInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7550_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7550_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7550_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static uint32_t BINT_P_7550_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );
static uint32_t GetRaveIntEnableOffset( uint32_t BaseAddr );

static const BINT_P_IntMap bint_7550[] =
{
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_PB0_INTR,                          0,                      "XPT_PB0"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_PB1_INTR,                          0,                      "XPT_PB1"},
    
    BINT_MAP(0, XPT_RAV, "_CX0",XPT_RAVE_INT_CX0, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX1",XPT_RAVE_INT_CX1, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX2",XPT_RAVE_INT_CX2, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX3",XPT_RAVE_INT_CX3, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX4",XPT_RAVE_INT_CX4, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX5",XPT_RAVE_INT_CX5, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX6",XPT_RAVE_INT_CX6, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX7",XPT_RAVE_INT_CX7, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX8",XPT_RAVE_INT_CX8, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX9",XPT_RAVE_INT_CX9, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX10",XPT_RAVE_INT_CX10, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX11",XPT_RAVE_INT_CX11, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX12",XPT_RAVE_INT_CX12, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX13",XPT_RAVE_INT_CX13, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX14",XPT_RAVE_INT_CX14, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX15",XPT_RAVE_INT_CX15, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX16",XPT_RAVE_INT_CX16, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX17",XPT_RAVE_INT_CX17, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX18",XPT_RAVE_INT_CX18, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX19",XPT_RAVE_INT_CX19, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX20",XPT_RAVE_INT_CX20, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX21",XPT_RAVE_INT_CX21, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX22",XPT_RAVE_INT_CX22, WEAK_MASK, ALL, 0),
    BINT_MAP(0, XPT_RAV, "_CX23",XPT_RAVE_INT_CX23, WEAK_MASK, ALL, 0),
    
/*    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_RAV_CPU_INTR_SHIFT,             BCHP_XPT_RAVE_INT_MISC,                     0,                   "RAV_MISC"}, */

    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_ICAM_CPU_INTR_SHIFT,            BCHP_XPT_BUS_IF_INTR_STATUS3_REG ,          0,                   "XPT_ICAM"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_ICAM_CPU_INTR_SHIFT,            BCHP_XPT_BUS_IF_MISC_CTRL0,                 0x2,                   "XPT_ICAM"},
    
    /* TODO: Mapping of following bits to L2 register is missing in RDB.
       XPT_STATUS_CPU_INTR, XPT_RAV_CPU_INTR, XPT_ICAM_CPU_INTR */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_AIO_CPU_INTR_SHIFT,                 BCHP_AIO_INTH_R5F_STATUS,                   0,                      "AIO"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_RPTD_CPU_INTR_SHIFT,                BCHP_AUD_DSP_INTH0_R5F_STATUS,              0,                      "RPTD"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_DVP_HT_CPU_INTR_SHIFT,              BCHP_HDMI_INTR2_CPU_STATUS,                 0,                      "HDMI"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_VEC_CPU_INTR_SHIFT,                 BCHP_VIDEO_ENC_INTR2_CPU_STATUS,            0,                      "VEC"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNB_CPU_INTR_SHIFT,                BCHP_BVNB_INTR2_CPU_STATUS,                 0,                      "BVNB"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_CPU_INTR_0_SHIFT,              BCHP_BVNF_INTR2_0_R5F_STATUS,               0,                      "BVNF_0"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_CPU_INTR_1_SHIFT,              BCHP_BVNF_INTR2_1_R5F_STATUS,               0,                      "BVNF_1"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_CPU_INTR_5_SHIFT,              BCHP_BVNF_INTR2_5_R5F_STATUS,               0,                      "BVNF_3"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_MEMC_0_CPU_INTR_SHIFT,             BCHP_MEMC_L2_0_R5F_STATUS,		0,    "MEMC"},
/*
BCHP_IRQ0_IRQEN notes:

The IRQ0 L2 has multiple L1's tied to it. Each L1 has a separate entry in this table and only those L2's which apply to that L1 are marked valid.

uarta, uartb, uartc are the unshared interrupts. They are used by the OS. The URT PI does not use them. These should be marked invalid.
ua, ub, uc are the shared interrupts. They are used by the URT PI. They shuold be marked valid. They do not conflict with the unshared interrupts.
 */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_CPU_INTR_SHIFT,                 BCHP_IRQ0_IRQEN,                            0,                      "UPG"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_GPIO_CPU_INTR_SHIFT,            BCHP_IRQ0_GPIO_IRQEN,                       0,                      "GIO"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_TMR_CPU_INTR_SHIFT,             BCHP_TIMER_TIMER_IS,                        0,                      "TMR"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_BSC_CPU_INTR_SHIFT,             BCHP_IRQ0_BSC_IRQEN,                        0,                      "BSC"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_SPI_CPU_INTR_SHIFT,             BCHP_IRQ0_SPI_IRQEN,                        0,                      "SPI"},
    /* Following bits are used by OS. Hence not mapped.
       UPG_UART0_CPU_INTR, UPG_UART1_CPU_INTR, UPG_UART2_CPU_INTR, USB_CPU_INTR,
       USB_EHCI_CPU_INTR,USB_OHCI_CPU_INTR, MEMC_0_CPU, ENET_EMAC1_CPU */
    /* Following bits are not mapped.
       IPI0_CPU_INTR, IPI1_CPU_INTR */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_SC_CPU_INTR_SHIFT,              BCHP_SCIRQ0_SCIRQEN,                        0,                      "SC"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_MC_CPU_INTR_SHIFT,                  BCHP_MCIF_INTR2_CPU_STATUS,                 0,                      "MC"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_SUN_CPU_INTR_SHIFT,                 BCHP_SUN_L2_CPU_STATUS,                     0xFFD0003E,             "SUN"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_AVD0_CPU_INTR_SHIFT,                BCHP_AVD_INTR2_0_CPU_STATUS,                0,                      "AVD_0"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_DPCR0_INTR_STATUS_REG,             0,                      "XPT_DPCR0"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS,        0,                      "XPT_DPCR0"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_FE_CPU_INTR_SHIFT + 32,         BCHP_XPT_FE_INTR_STATUS_REG,                0,                      "XPT_FE"},
    { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_THD_CPU_INTR_SHIFT + 64,            BCHP_THD_INTR2_CPU_STATUS,                  0,                      "THD"},
    { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_DS0_CPU_INTR_0_SHIFT + 64,          BCHP_DS_TUNER_IRQSTS,                       0,                      "DS0"},
    { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_DS1_CPU_INTR_1_SHIFT + 64,          BCHP_DS_IRQSTS1,                            0,                      "DS1"},
    { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_DS2_CPU_INTR_2_SHIFT + 64,          BCHP_DS_IRQSTS2,                            0,                      "DS2"},
#if BCHP_VER >= BCHP_VER_B0
    { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_M2MC_CPU_INTR_SHIFT + 64,           BCHP_M2MC_TOP_L2_CPU_STATUS,                0,                      "M2MC"},
    { BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_SHARF_CPU_INTR_SHIFT + 64,          BCHP_SHARF_INTR2_CPU_STATUS,                0,                      "SHARF"},
#endif    
    { -1, 0, 0, NULL}
};

static const BINT_Settings bint_7550Settings =
{
    BINT_P_7550_SetInt,
    BINT_P_7550_ClearInt,
    BINT_P_7550_SetMask,
    BINT_P_7550_ClearMask,
    NULL,
    BINT_P_7550_ReadStatus,
    bint_7550,
    "7550"
};

static void BINT_P_7550_SetInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    switch( baseAddr )
    {
    BINT_P_STANDARD_L2_CASES
        BREG_Write32( regHandle, baseAddr + BINT_P_STD_SET, 1ul<<shift);
        break;
    default:
        /* Only standard L2 interrupts support setting of interrupts */
        break;
    }
}

static void BINT_P_7550_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    BDBG_MSG(("ClearInt %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
        BINT_P_STANDARD_L2_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_STD_CLEAR, 1ul<<shift);
            break;
        BINT_P_XPT_STATUS_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_XPT_STATUS, ~(1ul<<shift));
            break;
        BINT_P_XPT_RAVE_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_RAVE_STATUS, (1ul<<shift));
            break;
        BINT_P_TIMER_CASES
            BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_STATUS, 1ul<<shift);
            break;
        BINT_P_UPGSC_CASES
        BINT_P_IRQ0_CASES
            /* Has to cleared at the source */
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

static void BINT_P_7550_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;

    uint32_t RaveEnReg = 0;

    BDBG_MSG(("SetMask %#x:%d", baseAddr, shift));

    switch( baseAddr )
    {
    BINT_P_STANDARD_L2_CASES
        BREG_Write32( regHandle, baseAddr + BINT_P_STD_MASK_SET, 1ul<<shift);
        break;
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
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable);
        break;
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

static void BINT_P_7550_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
{
    uint32_t intEnable;

    uint32_t RaveEnReg = 0;

    BDBG_MSG(("ClearMask %#x:%d", baseAddr, shift));
    switch( baseAddr )
    {
    BINT_P_STANDARD_L2_CASES
        BREG_Write32( regHandle, baseAddr + BINT_P_STD_MASK_CLEAR, 1ul<<shift);
        break;
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

static uint32_t BINT_P_7550_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
{
    BDBG_MSG(("ReadStatus %#x", baseAddr));
    switch( baseAddr )
    {
    BINT_P_STANDARD_L2_CASES
        return BREG_Read32(regHandle, baseAddr + BINT_P_STD_STATUS);
    BINT_P_XPT_STATUS_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_XPT_STATUS );
    BINT_P_XPT_RAVE_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_RAVE_STATUS );
    BINT_P_TIMER_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_STATUS );
    BINT_P_IRQ0_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_STATUS );
    BINT_P_UPGSC_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_UPGSC_ENABLE );
    BINT_P_PCROFFSET_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_STATUS );
    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        return 0;
    }
}

const BINT_Settings *BINT_7550_GetSettings( void )
{
    return &bint_7550Settings;
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

