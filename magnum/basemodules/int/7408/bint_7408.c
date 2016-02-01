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
 ***************************************************************************/

#include "bstd.h"
#include "bint_7408.h"

/* Include interrupt definitions from RDB */
#include "bchp_hif_cpu_intr1.h"

/* Standard L2 stuff */
#include "bchp_aio_inth.h"
#include "bchp_avd_intr2_0.h"

#include "bchp_bvnb_intr2.h"
#include "bchp_bvnf_intr2_0.h"
#include "bchp_bvnf_intr2_1.h"
#include "bchp_bvnf_intr2_3.h"
#include "bchp_bvnf_intr2_4.h"
#include "bchp_bvnf_intr2_5.h"
#include "bchp_bvnf_intr2_6.h"

#include "bchp_gfx_l2.h"

#include "bchp_memc_l2_0.h"
#include "bchp_hdmi_intr2.h"
#include "bchp_mcif_intr2.h"
#include "bchp_uhfr_1.h"
#include "bchp_sun_l2.h"
#include "bchp_usb_intr2.h"
#include "bchp_video_enc_intr2.h"

/* XPT */
#include "bchp_xpt_fe.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_dpcr0.h"
#include "bchp_xpt_dpcr1.h"
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_rave.h"
#include "bchp_xpt_pcroffset.h"

/* UARTs, keypad, I2C */
#include "bchp_irq0.h"

/* Timer */
#include "bchp_timer.h"

#include "bchp_ipi0_intr2.h"

BDBG_MODULE(interruptinterface_7408);

#define BINT_P_STD_STATUS       0x00
#define BINT_P_STD_SET          0x04
#define BINT_P_STD_CLEAR        0x08
#define BINT_P_STD_MASK_STATUS  0x0C
#define BINT_P_STD_MASK_SET     0x10
#define BINT_P_STD_MASK_CLEAR   0x14


#define BINT_P_STANDARD_L2_CASES            \
    /* BVN_TOP */                           \
    case BCHP_BVNF_INTR2_0_R5F_STATUS:      \
    case BCHP_BVNF_INTR2_1_R5F_STATUS:      \
    case BCHP_BVNF_INTR2_3_AVD0_STATUS:     \
    case BCHP_BVNF_INTR2_4_AVD0_STATUS:     \
    case BCHP_BVNF_INTR2_5_R5F_STATUS:      \
    case BCHP_BVNF_INTR2_6_R5F_STATUS:      \
    case BCHP_BVNB_INTR2_CPU_STATUS:        \
    /* VIDEO_ENC_BLOCK */                   \
    case BCHP_VIDEO_ENC_INTR2_CPU_STATUS:   \
    /* HDMI_BLOCK */                        \
    case BCHP_HDMI_INTR2_CPU_STATUS:        \
    /* XPT FE */                            \
    case BCHP_XPT_FE_INTR_STATUS_REG:       \
    /* SUNDRY */                            \
    case BCHP_MCIF_INTR2_CPU_STATUS:        \
    case BCHP_SUN_L2_CPU_STATUS:            \
    /* AUD_ALL */                           \
    case BCHP_AIO_INTH_R5F_STATUS:          \
    /* AVD */                               \
    case BCHP_AVD_INTR2_0_CPU_STATUS:       \
   /* MEMC */                               \
   case BCHP_MEMC_L2_0_R5F_STATUS:          \
   /* GRAPHICS */                           \
   case BCHP_GFX_L2_CPU_STATUS:             \
   case BCHP_IPI0_INTR2_CPU_STATUS:

    

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
    case BCHP_XPT_DPCR0_INTR_STATUS_REG: \
    case BCHP_XPT_DPCR1_INTR_STATUS_REG:

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


#define BINT_P_TIMER_STATUS     0x00
#define BINT_P_TIMER_MASK       0x04

#define BINT_P_TIMER_CASES \
    case BCHP_TIMER_TIMER_IS:

#define BINT_P_UHF_STATUS	    0x04
#define BINT_P_UHF_MASK		    0x00

#define BINT_P_UHF_CASES \
	case BCHP_UHFR_1_IMSK:

#define BINT_P_STAT_TIMER_TICKS_PER_USEC 27

static void BINT_P_7408_SetInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7408_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7408_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static void BINT_P_7408_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift );
static uint32_t BINT_P_7408_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr );
static uint32_t GetRaveIntEnableOffset( uint32_t BaseAddr );

static const BINT_P_IntMap bint_7408[] =
{
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

    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_AVD0_CPU_INTR_SHIFT,                BCHP_AVD_INTR2_0_CPU_STATUS,           0,                   "AVD_0"},

    /* { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_CPU_INTR_5_SHIFT,              BCHP_BVNF_INTR2_5_R5F_STATUS,          0,                   "BVNF_3"}, */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_5_CPU_INTR_SHIFT,              BCHP_BVNF_INTR2_5_R5F_STATUS,          0,                   "BVNF_3"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_MC_CPU_INTR_SHIFT,                  BCHP_MCIF_INTR2_CPU_STATUS,            0xFFFC07FF,          "BICAP"},

    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UHF1_CPU_INTR_SHIFT,                BCHP_UHFR_1_IMSK,                      0,                   "UFHR1"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_SUN_CPU_INTR_SHIFT,                 BCHP_SUN_L2_CPU_STATUS,                0xFFF0003E,          "SUN"},
    
    /* iica_irqen,iicb_irqen,iicc_irqen,iicd_irqen */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_BSC_CPU_INTR_SHIFT,             BCHP_IRQ0_IRQEN,                       0xF0FFFFFF,          "BSC"},
    /* kbd1_irqen,ldk_irqen,irb_irqen,ua_irqen,kbd2_irqen,gio_irqen,icap_irqen,kbd3_irqen */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_CPU_INTR_SHIFT,                 BCHP_IRQ0_IRQEN,                       0xFFFFFC00,          "UPG"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_UPG_TMR_CPU_INTR_SHIFT,             BCHP_TIMER_TIMER_IS,                   0,                   "TMR"},
    
    /* main memory controller block has become independent from graphics */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_MEMC_0_CPU_INTR_SHIFT,              BCHP_MEMC_L2_0_R5F_STATUS,             0,                   "MEMC_0"},

/*
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_CPU_INTR_1_SHIFT,              BCHP_BVNF_INTR2_1_R5F_STATUS,          0,                   "BVNF_1"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_CPU_INTR_0_SHIFT,              BCHP_BVNF_INTR2_0_R5F_STATUS,          0,                   "BVNF_0"},
*/
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_1_CPU_INTR_SHIFT,              BCHP_BVNF_INTR2_1_R5F_STATUS,          0,                   "BVNF_1"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNF_0_CPU_INTR_SHIFT,              BCHP_BVNF_INTR2_0_R5F_STATUS,          0,                   "BVNF_0"},

    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_BVNB_CPU_INTR_SHIFT,                BCHP_BVNB_INTR2_CPU_STATUS,            0,                   "BVNB"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_VEC_CPU_INTR_SHIFT,                 BCHP_VIDEO_ENC_INTR2_CPU_STATUS,       0,                   "VEC"},  
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_HDMI_CPU_INTR_SHIFT,                BCHP_HDMI_INTR2_CPU_STATUS,            0,                   "HDMI"},
    /* GR_BRIDGE_INTR,M2MC_0_INTR,MEM_DMA_0_INTR */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_GFX_CPU_INTR_SHIFT,                 BCHP_GFX_L2_CPU_STATUS,		   0,                   "GFX"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_AIO_CPU_INTR_SHIFT,                 BCHP_AIO_INTH_R5F_STATUS,              0,                   "AIO"},        
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_BUS_IF_INTR_STATUS_REG,       0,                   "XPT"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_MDMA_CPU_INTR_SHIFT,            BCHP_XPT_BUS_IF_INTR_STATUS2_REG,      0,                   "XPT2"},
    /* { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_BUS_IF_INTR_STATUS2_REG,      0,                   "XPT2"}, */
    /* { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_BUS_IF_INTR_STATUS3_REG,      0,                   "XPT3"}, */
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_BUS_IF_INTR_STATUS4_REG,      0,                   "XPT4"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_BUS_IF_INTR_STATUS5_REG,      0,                   "XPT5"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_PB0_INTR,                     0,                   "XPT_PB0"},
    { BCHP_HIF_CPU_INTR1_INTR_W0_STATUS_XPT_STATUS_CPU_INTR_SHIFT,          BCHP_XPT_PB1_INTR,                     0,                   "XPT_PB1"},

    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_IPI0_CPU_INTR_SHIFT   + 32,         BCHP_IPI0_INTR2_CPU_STATUS,            0,                   "IPI0"},
    
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_FE_CPU_INTR_SHIFT + 32,         BCHP_XPT_FE_INTR_STATUS_REG,           0,                   "XPT_FE"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_DPCR0_INTR_STATUS_REG,        0,                   "XPT_DPCR0"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_DPCR1_INTR_STATUS_REG,        0,                   "XPT_DPCR1"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS,  0,                   "XPT_PCROFF0"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS,  0,                   "XPT_PCROFF1"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS,  0,                   "XPT_PCROFF2"},
    { BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_XPT_PCR_CPU_INTR_SHIFT + 32,        BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS,  0,                   "XPT_PCROFF3"},
    { -1, 0, 0, NULL} 
};

static const BINT_Settings bint_7408Settings =
{
    BINT_P_7408_SetInt,
    BINT_P_7408_ClearInt,
    BINT_P_7408_SetMask,
    BINT_P_7408_ClearMask,
    NULL,
    BINT_P_7408_ReadStatus, 
    bint_7408,
    "7408"
};

static void BINT_P_7408_SetInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
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

static void BINT_P_7408_ClearInt( BREG_Handle regHandle, uint32_t baseAddr, int shift )
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
	BINT_P_UHF_CASES
            /* Interrupt is cleared twice for UHF interrupt.
               Richa needs to look into this for real solution.
               BREG_Write32( regHandle, baseAddr + BINT_P_UHF_STATUS, 1ul<<shift); */
            break;
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

static void BINT_P_7408_SetMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
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
    BINT_P_UHF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UHF_MASK);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_UHF_MASK, intEnable);
        break;
    BINT_P_IRQ0_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE);
        intEnable &= ~(1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_IRQ0_ENABLE, intEnable);
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

static void BINT_P_7408_ClearMask( BREG_Handle regHandle, uint32_t baseAddr, int shift )
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
    BINT_P_TIMER_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_TIMER_MASK );
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_TIMER_MASK, intEnable );
        break;
    BINT_P_UHF_CASES
        intEnable = BREG_Read32( regHandle, baseAddr + BINT_P_UHF_MASK);
        intEnable |= (1ul<<shift);
        BREG_Write32( regHandle, baseAddr + BINT_P_UHF_MASK, intEnable);
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

static uint32_t BINT_P_7408_ReadStatus( BREG_Handle regHandle, uint32_t baseAddr )
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
    BINT_P_UHF_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_UHF_STATUS );
    BINT_P_IRQ0_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_IRQ0_STATUS ); 
    BINT_P_PCROFFSET_CASES
        return BREG_Read32( regHandle, baseAddr + BINT_P_PCROFFSET_STATUS );
    default:
        /* Unhandled interrupt base address */
        BDBG_ASSERT( false );
        return 0;
    }
}

const BINT_Settings *BINT_7408_GetSettings( void )
{
    return &bint_7408Settings;
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

