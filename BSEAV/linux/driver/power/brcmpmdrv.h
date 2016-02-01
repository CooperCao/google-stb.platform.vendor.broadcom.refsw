/***************************************************************************
 *     Copyright (c) 2002-2010, Broadcom Corporation
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
 *  Test driver for Linux powerdown mode
 *
 * Design:
 * Insmod-able driver with a single SLEEP IOCTL.
 * When invoked it will substitute a body of code for the main MIPS interrupt vector.
 * The body of code is designed to run entirely in cache.
 * The body of code will manage a timer in the UPG block to maintain time of day.
 * The SLEEP IOCTL can accept a parameter specifying a time of day/date when the system is to be woken up.
 * The body of code will manage 2 other interrupts: IR receive and GPIO
 * If the code detects a valid IR code it will wake up the system
 * The SLEEP IOCTL can accept a parameter specifying which GPIO's (front panel buttons) will cause the system to wake up.
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/ 
#ifndef _BRCMPMDRV_H_
#define _BRCMPMDRV_H_

#ifdef _BRCMPMDRV_INTERNAL_
#include "bchp.h"

#if BCHP_CHIP == 7400

#include "bchp_memc_0_ddr.h"
#include "bchp_memc_0.h"

#ifndef __ASSEMBLY__
#include "bint_7400.h"
#endif

#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,  0,          01111111111 }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,  0,          01111111111 }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,  ~(07777),       01111 }, \
    }

#define CLIENT_EXCLUSION_LIST   {4, 11, 41, 49, 50, 59, 60, 61, -1}
#define REFRESH_CLIENT      63

#define SCB_CLIENT_BASE     BCHP_MEMC_0_CLIENT_INFO_0
#define DDR_PAD_REG     BCHP_MEMC_0_DDR_SPARE1_RW
#define DDR_PAD_MASK        0x2
#define B_MAX_REG_ADDRESS   BCHP_PX3D_REG_END
#define BCHP_OPEN       BCHP_Open7400
#define BINT_GETSETTINGS    BINT_7400_GetSettings
#define LDK_FP          1
#define SPI_FP          0
#define VGA_INPUT       0
#define HW_STANDBY      0
#define UART_3250       0

#define UART_BASE       BCHP_UARTA_THR
#define UART_LSR_OFFSET     0x14
#define UART_LSR_MASK       BCHP_UARTA_LSR_THRE_MASK
#define UART_TX_OFFSET      0x00

#elif BCHP_CHIP == 7405

#include "bchp_memc_0_ddr.h"
#include "bchp_memc_0.h"
#include "bchp_memc_0_1.h"
#include "bchp_ddr_iobuf.h"

#ifndef __ASSEMBLY__
#include "bint_7405.h"
#endif

#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,  ~(077 << 26),       (011 << 26) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,  0,          01111111111 }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,  ~(077777 << 0),     (011111 << 0) }, \
    }

#define CLIENT_EXCLUSION_LIST   {41, 121, 122, 123, 124, 125, 126, -1}
#define REFRESH_CLIENT      127

#define SCB_CLIENT_BASE     BCHP_MEMC_0_1_CLIENT_INFO_0
#define DDR_PAD_REG     BCHP_DDR_IOBUF_DDR_PAD_CNTRL
#define DDR_PAD_MASK        0x1
#define B_MAX_REG_ADDRESS   BCHP_MEM_DMA_SECURE_REG_END
#define BCHP_OPEN       BCHP_Open7405
#define BINT_GETSETTINGS    BINT_7405_GetSettings
#define LDK_FP          1
#define SPI_FP          0
#define VGA_INPUT       0
#define HW_STANDBY      0
#define UART_3250       0

#define UART_BASE       BCHP_UARTA_THR
#define UART_LSR_OFFSET     0x14
#define UART_LSR_MASK       BCHP_UARTA_LSR_THRE_MASK
#define UART_TX_OFFSET      0x00

#elif BCHP_CHIP == 7325

#include "bchp_memc_0_ddr.h"
#include "bchp_memc_0.h"
#include "bchp_memc_0_1.h"
#include "bchp_ddr_iobuf.h"

#ifndef __ASSEMBLY__
#include "bint_7325.h"
#endif

/* SPI front panel: gpio_31/ext_irqb_10 for keypress detect, gpio_71 for IR */
#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,   ~(07 << 9),     (01 << 9) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,   ~(03 << 3),     (01 << 3) }, \
    }

#define CLIENT_EXCLUSION_LIST   {41, 124, 125, 126, -1}
#define REFRESH_CLIENT      127

#define SCB_CLIENT_BASE     BCHP_MEMC_0_1_CLIENT_INFO_0
#define DDR_PAD_REG     BCHP_DDR_IOBUF_DDR_PAD_CNTRL
#define DDR_PAD_MASK        0x1
#define B_MAX_REG_ADDRESS   BCHP_FTM_INTR2_REG_END
#define BCHP_OPEN       BCHP_Open7325
#define BINT_GETSETTINGS    BINT_7325_GetSettings
#define LDK_FP          0
#define SPI_FP          1
#define VGA_INPUT       0
#define HW_STANDBY      0
#define UART_3250       0

#define SPI_IRQ_REG_STATUS  BCHP_HIF_CPU_INTR1_INTR_W1_STATUS
#define SPI_IRQ_REG_SET     BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET
#define SPI_IRQ_REG_CLEAR   BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR
#define SPI_IRQ_MASK        BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET_EXT_IRQ_10_CPU_INTR_MASK

#define SPI_IRQ_LEVEL_REG   BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL
#define SPI_IRQ_LEVEL_MASK  (~0)
#define SPI_IRQ_LEVEL_VAL   (BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_10_level_MASK)

#define UART_BASE       BCHP_UARTA_THR
#define UART_LSR_OFFSET     0x14
#define UART_LSR_MASK       BCHP_UARTA_LSR_THRE_MASK
#define UART_TX_OFFSET      0x00

#elif BCHP_CHIP == 7335

#include "bchp_memc_0_ddr.h"
#include "bchp_memc_0.h"
#include "bchp_memc_0_1.h"
#include "bchp_ddr_iobuf.h"

#ifndef __ASSEMBLY__
#include "bint_7335.h"
#endif

/* SPI front panel: gpio_017/ext_irqb_5 for keypress detect, ir_in0 for IR */
#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,   ~(07 << 18),        (03 << 18) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,   ~(07 << 12),        (00 << 12) }, \
    }

#define CLIENT_EXCLUSION_LIST   {41, 124, 125, -1}
#define REFRESH_CLIENT      127

#define SCB_CLIENT_BASE     BCHP_MEMC_0_1_CLIENT_INFO_0
#define DDR_PAD_REG     BCHP_DDR_IOBUF_DDR_PAD_CNTRL
#define DDR_PAD_MASK        0x1
#define B_MAX_REG_ADDRESS   BCHP_FTM_INTR2_REG_END
#define BCHP_OPEN       BCHP_Open7335
#define BINT_GETSETTINGS    BINT_7335_GetSettings
#define LDK_FP          0
#define SPI_FP          1
#define VGA_INPUT       0
#define HW_STANDBY      0
#define UART_3250       0

#define SPI_IRQ_REG_STATUS  BCHP_HIF_CPU_INTR1_INTR_W1_STATUS
#define SPI_IRQ_REG_SET     BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET
#define SPI_IRQ_REG_CLEAR   BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR
#define SPI_IRQ_MASK        BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET_EXT_IRQ_5_CPU_INTR_MASK

#define SPI_IRQ_LEVEL_REG   BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL
#define SPI_IRQ_LEVEL_MASK  (~0)
#define SPI_IRQ_LEVEL_VAL   (BCHP_HIF_TOP_CTRL_EXT_IRQ_LEVEL_ext_irq_5_level_MASK)

#elif (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)

#if BCHP_CHIP == 3548

#ifndef __ASSEMBLY__
#include "bint_3548.h"
#endif
#define BCHP_OPEN       BCHP_Open3548
#define BINT_GETSETTINGS    BINT_3548_GetSettings

#else

#ifndef __ASSEMBLY__
#include "bint_3556.h"
#endif
#define BCHP_OPEN       BCHP_Open3556
#define BINT_GETSETTINGS    BINT_3556_GetSettings

#endif

#include "bchp_memc_0_1.h"

#if BCHP_VER == BCHP_VER_A0
#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,   ~(07 << 0), (02 << 0) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,   ~(07 << 9), (02 << 9) }, \
    }
#else /* BCHP_VER == BCHP_VER_A0 */
#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,   ~(07 <<  7),    (02 <<  7) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,   ~(07 << 10),    (02 << 10) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,   ~(07 << 18),    (02 << 18) }, \
      { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,   ~(07 << 29),    (02 << 29) }, \
    }
#endif /* BCHP_VER == BCHP_VER_A0 */

#define CLIENT_EXCLUSION_LIST   {41, 42, 48, 123, 124, 125, 126, -1}
#define REFRESH_CLIENT      127

#define SCB_CLIENT_BASE     BCHP_MEMC_0_1_CLIENT_INFO_0
#define B_MAX_REG_ADDRESS   BCHP_PX3D_REG_END

#define LDK_FP          0
#define SPI_FP          1
#define VGA_INPUT       1
#define HW_STANDBY      1
#define BUGGY_WKTMR     1
#define UART_3250       1
#define HDMI_CEC_FEATURE  1

#define SPI_IRQ_MASK        BCHP_CG_INTR2_CPU_STATUS_EXT_IRQ_B_2_INTR_MASK
#define SPI_EXT_IRQ_B       2

#define VGA_HSYNC_IRQ_MASK  BCHP_CG_INTR2_CPU_STATUS_EXT_IRQ_B_3_INTR_MASK
#define VGA_VSYNC_IRQ_MASK  BCHP_CG_INTR2_CPU_STATUS_EXT_IRQ_B_1_INTR_MASK

/*
 * Minimum VGA DPMS mode to wake on:
 *  3 - On
 *  2 - Standby
 *  1 - Suspend
 */
#define VGA_WAKEUP_MODE     3

#elif BCHP_CHIP == 7125

#include "bchp_memc_ddr_0.h"
#include "bchp_memc_arb_0.h"

#ifndef __ASSEMBLY__
#include "bint_7125.h"
#endif
/* todo pinmux */
/* Small board has no keypad, so only gpio_29 for IR_IN0 */
#define PINMUX_SETUP        \
    /*  register                mask            value */ \
    { { BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,   ~(03 << 24),     (01 << 24) }, }

#define CLIENT_EXCLUSION_LIST   {41, 121, 122, 123, 124, 125, 126, -1}
#define REFRESH_CLIENT      127

#define SCB_CLIENT_BASE     BCHP_MEMC_ARB_0_CLIENT_INFO_0
#if 0
#define DDR_PAD_REG     BCHP_DDR_IOBUF_DDR_PAD_CNTRL
#define DDR_PAD_MASK        0x1
#endif
#define B_MAX_REG_ADDRESS   BCHP_IN656_0_REG_END+4
#define BCHP_OPEN       BCHP_Open7125
#define BINT_GETSETTINGS    BINT_7125_GetSettings
#define LDK_FP          1
#define SPI_FP          0
#define VGA_INPUT       0
#define HW_STANDBY      0
#define UART_3250       0

#define UART_BASE       BCHP_UARTA_THR
#define UART_LSR_OFFSET     0x14
#define UART_LSR_MASK       BCHP_UARTA_LSR_THRE_MASK
#define UART_TX_OFFSET      0x00

#endif

#if ! UART_3250
#define UART_BASE       BCHP_UARTA_THR
#define UART_LSR_OFFSET     0x14
#define UART_LSR_MASK       BCHP_UARTA_LSR_THRE_MASK
#define UART_TX_OFFSET      0x00
#else
#define UART_BASE       BCHP_UARTA_RCVSTAT
#define UART_LSR_OFFSET     0x18
#define UART_LSR_MASK       BCHP_UARTA_XMTSTAT_TDRE_MASK
#define UART_TX_OFFSET      0x1c
#endif


#if ! HW_STANDBY
#define PPMD_L2_ENABLE(handle, val) do { \
    BREG_Write32(handle, BCHP_IRQ0_IRQEN, \
        BREG_Read32(handle, BCHP_IRQ0_IRQEN) | (val)); \
    } while(0)
#define PPMD_L2_STATUS(handle, val) \
    (BREG_Read32(handle, BCHP_IRQ0_IRQSTAT) & (val))
#define PPMD_L2_CLEAR(handle, val)  do { } while(0)

#define PPMD_L2_MASK_KPD        BCHP_IRQ0_IRQEN_ldk_irqen_MASK
#define PPMD_L2_MASK_IR     BCHP_IRQ0_IRQEN_kbd1_irqen_MASK

/*
 * uncomment to enable wake on GPIO (you will need to change PINMUX_SETUP, and
 * add code to set up GIO_{IODIR,EC,EI,MASK,LEVEL})
 */
/* #define PPMD_L2_MASK_GPIO    BCHP_IRQ0_IRQEN_gio_irqen_MASK */
#define PPMD_L2_MASK_GPIO   0

#else /* ! HW_STANDBY */

#include "bchp_wktmr.h"
#include "bchp_cg_intr2.h"
#include "bchp_clkgen.h"
#include "bchp_pm.h"
#include "bchp_hif_top_ctrl.h"
#include "bchp_ddr_iobuf.h"

/* use the new power management controller (3548, 7420, and newer) */
#define PPMD_L2_ENABLE(handle, val) do { \
        BREG_Write32(handle, BCHP_CG_INTR2_CPU_CLEAR, (val)); \
        BREG_Write32(handle, BCHP_CG_INTR2_CPU_MASK_CLEAR, (val)); \
    } while(0)
#define PPMD_L2_STATUS(handle, val) \
    (BREG_Read32(handle, BCHP_CG_INTR2_CPU_STATUS) & (val))
#define PPMD_L2_CLEAR(handle, val)  do { \
        BREG_Write32(handle, BCHP_CG_INTR2_CPU_CLEAR, (val)); \
    } while(0)

#define PPMD_L2_MASK_KPD    BCHP_CG_INTR2_CPU_STATUS_KPD_INTR_MASK
#define PPMD_L2_MASK_IR     BCHP_CG_INTR2_CPU_STATUS_IRR_INTR_MASK

/*
 * Uncomment to enable wake on ext_irqb_0.  Supports ext_irqb_[0-6] ONLY.
 * Multiple ext_irqb_X bits may be ORed together here so that any one of
 * them is a wake event.  The L2 interrupt status will be returned to
 * the application so that it can determine which event occurred.
 *
 * These are level-triggered interrupts and the polarity is set through
 * BCHP_CLKGEN_PM_IRQ_INV .  By default the driver only configures the
 * bits it knows about (SPI keypad, WKTMR, IR, etc.).
 *
 * The pinmux needs to be configured to set the desired pins for
 * ext_irqb_X.  This can be done in PINMUX_SETUP or in the application.
 */

/* #define PPMD_L2_MASK_GPIO    (BCHP_CG_INTR2_CPU_STATUS_EXT_IRQ_B_0_INTR_MASK) */
#define PPMD_L2_MASK_GPIO   0

#define SPI_IRQ_REG_STATUS  BCHP_CG_INTR2_CPU_STATUS
#define SPI_IRQ_REG_SET     BCHP_CG_INTR2_CPU_MASK_SET
#define SPI_IRQ_REG_CLEAR   BCHP_CG_INTR2_CPU_MASK_CLEAR
#define SPI_IRQ_LEVEL_REG   BCHP_CLKGEN_PM_IRQ_INV

/*
 * Pin 7 of SPI keypad connector - active low
 * For active high, swap 1 and 0 in MASK and VAL
 * To use a different pin, change SPI_IRQ_MASK and SPI_EXT_IRQ_B above
 */
#define SPI_IRQ_LEVEL_MASK  (~(1 << (5 + SPI_EXT_IRQ_B)))
#define SPI_IRQ_LEVEL_VAL   (0 << (5 + SPI_EXT_IRQ_B))

#define PPMD_L2_MASK_VGA    (VGA_HSYNC_IRQ_MASK | VGA_VSYNC_IRQ_MASK)

#endif /* ! HW_STANDBY */


#define CLIENT_BLOCK_VALUE  0x1fff00
#define REFRESH_BLOCK_VALUE 0xffff00
#define REFRESH_CLIENT_INFO (REG_OFFSET | SCB_CLIENT_BASE + REFRESH_CLIENT * 4)

#ifndef XTALFREQ
#define XTALFREQ		27000000
#endif

#endif /* _BRCMPMDRV_INTERNAL_ */

/*
 *      IOCTL structure and associated defines
 */
#define PPMD_IOCTL_DEFAULT_UPG_DURATION     2       /* Given in seconds */
#define PPMD_IOCTL_MAX_UPG_DURATION     39      /* Given in seconds */

#define PPMD_IOCTL_ACTIVATE_SLEEP       0x01        /* Start the sleep process */

#define PPMD_IOCTL_EVENT_IR         0x00000001
#define PPMD_IOCTL_EVENT_KEYPAD         0x00000002
#define PPMD_IOCTL_EVENT_GPIO           0x00000004
#define PPMD_IOCTL_EVENT_TIMEOUT        0x00000008
#define PPMD_IOCTL_EVENT_VGA            0x00000010
#define PPMD_IOCTL_EVENT_CEC            0x00000020

#define PPMD_IOCTL_FLAGS_MEMORY_SELF_REFRESH    0x00000010
#define PPMD_IOCTL_FLAGS_SELF_REFRESH_W_PADS    0x00000020
#define PPMD_IOCTL_FLAGS_USE_UPG_TIMER      0x00000040
#define PPMD_IOCTL_FLAGS_SET_UPG_TIMER_DURATION 0x00000080
#define PPMD_IOCTL_FLAGS_POWER_BUTTON_ONLY  0x00000100

#define PPMD_NO_SHUTDOWN            0
#define PPMD_SELF_REFRESH_ONLY          -1
#define PPMD_KILL_PADS              1

#ifndef __ASSEMBLY__
typedef struct _PPMD_IOCTL {
    /* input: ORed bitmask of PPMD_IOCTL_EVENT_* */
    unsigned long           events;

    /* input: ORed bitmask of PPMD_IOCTL_FLAGS_* */
    unsigned long           flags;

    /* for old platforms only */
    unsigned long           upg_timer_duration;

    /* input: wakeup time */
    struct timeval          end_time;

    /* input: CEC logical address */
    unsigned long           cec_addr;

    /* output: PPMD_IOCTL_EVENT_* for actual wakeup event */
    unsigned long           wake_event;

    /* output: CG_INTR2_CPU_STATUS showing ext_irqb_X status */
    unsigned long           ext_irq_mask;

    /* output: CEC command received */
    unsigned long           cec_cmd;
} PPMD_IOCTL;
#endif

#endif /* ! _BRCMPMDRV_H_ */
