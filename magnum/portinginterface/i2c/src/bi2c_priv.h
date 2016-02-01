/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#ifndef BI2C_PRIVATE_H__
#define BI2C_PRIVATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bi2c.h"
#include "breg_i2c_priv.h"
#include "bchp_pwr.h"
#if BCHP_IRQ0_REG_START
#include "bchp_int_id_irq0.h"
#elif BCHP_UPG_BSC_IRQ_REG_START
#include "bchp_int_id_upg_bsc_irq.h"
#endif
#if BCHP_UPG_BSC_AON_IRQ_REG_START
#include "bchp_int_id_upg_bsc_aon_irq.h"
#endif
#include "bchp_gio.h"
#include "bchp_sun_top_ctrl.h"
#if BCHP_AON_PIN_CTRL_REG_START
#include "bchp_aon_pin_ctrl.h"
#endif

#if BCHP_PWR_RESOURCE_HDMI_TX_CLK
#include "bchp_pwr_resources.h"
#endif

#if BCHP_HDMI_TX_AUTO_I2C_REG_START
#include "bchp_hdmi_tx_auto_i2c.h"
#include "bchp_int_id_hdmi_tx_scdc_intr2_0.h"
#define AUTO_I2C_ENABLED 1
#define BAUTO_I2C_BSC_CHANNEL 2
#endif

#if BCHP_IRQ0_AON_REG_START
    #include "bchp_aon_pin_ctrl.h"
    #include "bchp_irq0_aon.h"
    #include "bchp_int_id_irq0_aon.h"
#endif

#if BCHP_GIO_AON_REG_START
    #include "bchp_gio_aon.h"
#endif

#include "bkni_multi.h"

#if ((BCHP_CHIP==7601) && (BCHP_VER >= BCHP_VER_B0)) || (BCHP_CHIP==7635) || (BCHP_CHIP == 7630) || (BCHP_CHIP == 7420) || (BCHP_CHIP == 7125) || \
    (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) || (BCHP_CHIP == 7550) || (BCHP_CHIP == 7408) || (BCHP_CHIP == 7468) || (BCHP_CHIP == 7208) || \
    (BCHP_CHIP == 7422) || (BCHP_CHIP==7425) || (BCHP_CHIP==7435)
    #define BI2C_USES_SETUP_HDMI_HW_ACCESS
#endif

#if (BCHP_CHIP==7601) && (BCHP_VER >= BCHP_VER_B0)
    #define BI2C_USES_SET_SDA_DELAY
#endif

#define BI2C_P_MAX_TIMEOUT_MS 2000

#define BI2C_P_ACQUIRE_MUTEX(handle) BKNI_AcquireMutex((handle)->hMutex)
#define BI2C_P_RELEASE_MUTEX(handle) BKNI_ReleaseMutex((handle)->hMutex)

#define DEV_MAGIC_ID            ((BERR_I2C_ID<<16) | 0xFACE)

#define BI2C_CHK_RETCODE( rc, func )        \
do {                                        \
    if( (rc = BERR_TRACE(func)) != BERR_SUCCESS ) \
    {                                       \
        goto done;                          \
    }                                       \
} while(0)

#define MAX_I2C_READ_REQUEST            8
#define MAX_I2C_READ_NO_ADDR_REQUEST    8
#define MAX_I2C_WRITE_REQUEST           8
#define MAX_I2C_WRITE_NO_ADDR_REQUEST   8
#define I2C_POLLING_INTERVAL            50   /* in usecs */
#define I2C_RESET_INTERVAL              5    /* in milli secs */
#define I2C_MAX_RESET_COUNT             400  /* I2C_RESET_INTERVAL * I2C_MAX_RESET_COUNT =  2 seconds MAX timeout.   */

#define EDDC_SEGMENT_CHIP_ADDR          0x60

#ifdef BCHP_BSCA_CTLHI_REG_DATA_REG_SIZE_MASK
    #define BI2C_SUPPORT_4_BYTE_XFR_MODE
#endif

#define MAX_AUTO_I2C_READ_REQUEST          8

#define REGX_CFG_WR_EN                 0x100

#define BHDM_AUTO_I2C_P_TRIGGER_BY_SW      0
#define BHDM_AUTO_I2C_P_TRIGGER_BY_TIMER   1

#define BHDM_AUTO_I2C_P_ENABLE_SHIFT      27
#define BHDM_AUTO_I2C_P_TRIGGER_SRC_SHIFT 26
#define BHDM_AUTO_I2C_P_MODE_SHIFT        24

#define BHDM_AUTO_I2C_P_SCL_SEL_375        0
#define BHDM_AUTO_I2C_P_SCL_SEL_390        1
#define BHDM_AUTO_I2C_P_SCL_SEL_187        2
#define BHDM_AUTO_I2C_P_SCL_SEL_200        3

#define BHDM_AUTO_I2C_P_DIV_CLK_BY_1       0
#define BHDM_AUTO_I2C_P_DIV_CLK_BY_4       1

typedef enum BAUTO_I2C_BSCX_P_OFFSET
{
    BAUTO_I2C_BSCX_P_OFFSET_eCHIP_ADDR = 0,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN0,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN1,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN2,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN3,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN4,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN5,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN6,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_IN7,
    BAUTO_I2C_BSCX_P_OFFSET_eCNT_REG,
    BAUTO_I2C_BSCX_P_OFFSET_eCTL_REG,
    BAUTO_I2C_BSCX_P_OFFSET_eIIC_ENABLE,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT0,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT1,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT2,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT3,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT4,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT5,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT6,
    BAUTO_I2C_BSCX_P_OFFSET_eDATA_OUT7,
    BAUTO_I2C_BSCX_P_OFFSET_eCTLHI_REG,
    BAUTO_I2C_BSCX_P_OFFSET_eSCL_PARAM,
    BAUTO_I2C_BSCX_P_OFFSET_eMax
} BAUTO_I2C_BSCX_P_OFFSET;

#if AUTO_I2C_ENABLED
typedef enum BAUTO_I2C_P_BASE_ADDRESS
{
    BAUTO_I2C_P_BASE_ADDRESS_eCH0 = BCHP_HDMI_TX_AUTO_I2C_CH0_CFG,
    BAUTO_I2C_P_BASE_ADDRESS_eCH1 = BCHP_HDMI_TX_AUTO_I2C_CH1_CFG,
    BAUTO_I2C_P_BASE_ADDRESS_eCH2 = BCHP_HDMI_TX_AUTO_I2C_CH2_CFG,
    BAUTO_I2C_P_BASE_ADDRESS_eCH3 = BCHP_HDMI_TX_AUTO_I2C_CH3_CFG,
    BAUTO_I2C_P_BASE_ADDRESS_eCHMax
}  BAUTO_I2C_P_BASE_ADDRESS_CHX ;

typedef enum BAUTO_I2C_P_REG0_ADDRESS
{
    BAUTO_I2C_P_REG0_ADDRESS_eCH0 = BCHP_HDMI_TX_AUTO_I2C_CH0_REG0_CFG,
    BAUTO_I2C_P_REG0_ADDRESS_eCH1 = BCHP_HDMI_TX_AUTO_I2C_CH1_REG0_CFG,
    BAUTO_I2C_P_REG0_ADDRESS_eCH2 = BCHP_HDMI_TX_AUTO_I2C_CH2_REG0_CFG,
    BAUTO_I2C_P_REG0_ADDRESS_eCH3 = BCHP_HDMI_TX_AUTO_I2C_CH3_REG0_CFG,
    BAUTO_I2C_P_REG0_ADDRESS_eCHMax
}  BAUTO_I2C_P_REG0_ADDRESS ;
#endif

typedef enum BAUTO_I2C_P_CHX_REG_OFFSET
{
    BAUTO_I2C_P_CHX_REG_OFFSET_e0,
    BAUTO_I2C_P_CHX_REG_OFFSET_e1,
    BAUTO_I2C_P_CHX_REG_OFFSET_e2,
    BAUTO_I2C_P_CHX_REG_OFFSET_e3,
    BAUTO_I2C_P_CHX_REG_OFFSET_e4,
    BAUTO_I2C_P_CHX_REG_OFFSET_e5,
    BAUTO_I2C_P_CHX_REG_OFFSET_e6,
    BAUTO_I2C_P_CHX_REG_OFFSET_e7,
    BAUTO_I2C_P_CHX_REG_OFFSET_e8,
    BAUTO_I2C_P_CHX_REG_OFFSET_e9,
    BAUTO_I2C_P_CHX_REG_OFFSET_e10,
    BAUTO_I2C_P_CHX_REG_OFFSET_e11,
    BAUTO_I2C_P_CHX_REG_OFFSET_e12,
    BAUTO_I2C_P_CHX_REG_OFFSET_e13,
    BAUTO_I2C_P_CHX_REG_OFFSET_eMax
}  BAUTO_I2C_P_CHX_REG_OFFSET ;

typedef enum BAUTO_I2C_P_CHX
{
    BAUTO_I2C_P_CHX_CFG,
    BAUTO_I2C_P_CHX_REG0_CFG,
    BAUTO_I2C_P_CHX_REG0_WD,
    BAUTO_I2C_P_CHX_REG1_CFG,
    BAUTO_I2C_P_CHX_REG1_WD,
    BAUTO_I2C_P_CHX_REG2_CFG,
    BAUTO_I2C_P_CHX_REG2_WD,
    BAUTO_I2C_P_CHX_REG3_CFG,
    BAUTO_I2C_P_CHX_REG3_WD,
    BAUTO_I2C_P_CHX_REG4_CFG,
    BAUTO_I2C_P_CHX_REG4_WD,
    BAUTO_I2C_P_CHX_REG5_CFG,
    BAUTO_I2C_P_CHX_REG5_WD,
    BAUTO_I2C_P_CHX_REG6_CFG,
    BAUTO_I2C_P_CHX_REG6_WD,
    BAUTO_I2C_P_CHX_REG7_CFG,
    BAUTO_I2C_P_CHX_REG7_WD,
    BAUTO_I2C_P_CHX_REG8_CFG,
    BAUTO_I2C_P_CHX_REG8_WD,
    BAUTO_I2C_P_CHX_REG9_CFG,
    BAUTO_I2C_P_CHX_REG9_WD,
    BAUTO_I2C_P_CHX_REG10_CFG,
    BAUTO_I2C_P_CHX_REG10_WD,
    BAUTO_I2C_P_CHX_REG11_CFG,
    BAUTO_I2C_P_CHX_REG11_WD,
    BAUTO_I2C_P_CHX_REG12_CFG,
    BAUTO_I2C_P_CHX_REG12_WD,
    BAUTO_I2C_P_CHX_REG13_CFG,
    BAUTO_I2C_P_CHX_REG13_WD,
    BAUTO_I2C_P_CHX_RD_0,
    BAUTO_I2C_P_CHX_RD_1,
    BAUTO_I2C_P_CHX_STAT,
    BAUTO_I2C_P_CHX_eMax
}  BAUTO_I2C_P_CHX ;

/*******************************************************************************
*
*   Private Module Handles
*
*******************************************************************************/
/* Data Transfer Format */
typedef enum BI2C_P_MODE
{
    BI2C_P_MODE_eWriteOnly,
    BI2C_P_MODE_eReadOnly,
    BI2C_P_MODE_eReadWrite,
    BI2C_P_MODE_eWriteRead,
    B_I2C_P_MODE_eMax
} BI2C_P_MODE;

typedef enum BAUTO_I2C_P_MODE
{
    BAUTO_I2C_P_MODE_eWrite,
    BAUTO_I2C_P_MODE_eRead,
    BAUTO_I2C_P_MODE_ePollScdcUpdate0,
    BAUTO_I2C_P_MODE_ePollHdcp22RxStatus,
    BAUTO_I2C_P_MODE_eMax
} BAUTO_I2C_P_MODE ;

typedef struct BAUTO_I2C_P_TriggerConfiguration
{
    uint8_t enable;
    uint8_t triggerSource ;  /* RDB Write (0) or Timer (1) */
    BAUTO_I2C_P_MODE eMode ;
    BAUTO_I2C_BSCX_P_OFFSET readAddressOffset; /* Valid values are between eDATA_OUT0 and eDATA_OUT7 */
    uint8_t timerMs ;
} BAUTO_I2C_P_TriggerConfiguration ;

typedef struct BAUTO_I2C_P_IICEnableSettings
{
    bool restart;
    bool noStart;
    bool noStop;
    bool enable;
} BAUTO_I2C_P_IICEnableSettings ;

typedef struct BAUTO_I2C_P_ChxRegXSettings
{
    bool enable;
    BAUTO_I2C_BSCX_P_OFFSET writeAddressOffset;
    uint32_t data;
} BAUTO_I2C_P_ChxRegXSettings ;


typedef struct BAUTO_I2C_P_CtlhiSettings
{
    bool ignoreAck;
} BAUTO_I2C_P_CtlhiSettings;

typedef struct BAUTO_I2C_P_CtlSettings
{
    BI2C_P_MODE mode;
} BAUTO_I2C_P_CtlSettings;

typedef struct BI2C_gpio_P_params {
    unsigned long mask;
    unsigned long data;
    unsigned long iodir;
    unsigned long oden;
} BI2C_gpio_P_params;

typedef struct BI2C_soft_i2c_P_bus {
    BI2C_gpio_P_params scl;
    BI2C_gpio_P_params sda;
} BI2C_soft_i2c_P_bus;

typedef struct BI2C_P_ChannelHandle
{
	BDBG_OBJECT(BI2C_P_ChannelHandle)
    uint32_t        magicId;                    /* Used to check if structure is corrupt */
    BI2C_Handle     hI2c;
    unsigned int    chnNo;
    uint32_t        coreOffset;
    BKNI_EventHandle    hChnEvent;
    BINT_CallbackHandle hChnCallback;
    BKNI_MutexHandle hMutex;                    /* Mutex handle for serialization */

    bool                noAck;
    bool                nvramAck;
    unsigned int        timeoutMs;

    /* use auto i2c or not */
    struct {
        uint8_t   channelNumber;       /* Extend the auto i2c channels as bi2c channels. */
        uint32_t  baseAddress;
        uint32_t  reg0BaseAddress;
        uint32_t  currentRegOffset;
        BAUTO_I2C_P_TriggerConfiguration trigConfig;
    }autoI2c;

    BI2C_soft_i2c_P_bus softI2cBus;
	BI2C_ChannelSettings chnSettings;

	BLST_S_ENTRY(BI2C_P_ChannelHandle) link;
} BI2C_P_ChannelHandle;

typedef struct BI2C_P_Channel_Handle_Head BI2C_P_Channel_Handle_Head;
BLST_S_HEAD(BI2C_P_Channel_Handle_Head, BI2C_P_ChannelHandle);

typedef struct BI2C_P_Handle
{
    uint32_t        magicId;                    /* Used to check if structure is corrupt */
    BCHP_Handle     hChip;
    BREG_Handle     hRegister;
    BINT_Handle     hInterrupt;
    unsigned int    maxChnNo;

	/* link list for managed pin records */
	BI2C_P_Channel_Handle_Head  chnHandleHead;
    BI2C_ChannelHandle hI2cChn[BI2C_MAX_I2C_CHANNELS];
} BI2C_P_Handle;

/* Ack/Nack bits in I2C protocol */
#define I2C_ACK         0
#define I2C_NACK        1

/* user defined delay for I2C
int I2c_delay_count = 200;
*/

/***************************************************************************
Auto I2C private functions.

****************************************************************************/
void BAUTO_I2C_P_HandleInterrupt_Isr(
    void *pParam1,                      /* [in] Device handle */
    int parm2                           /* [in] not used */
);

void BAUTO_I2C_P_GetDefaultTriggerConfig(
    BAUTO_I2C_P_TriggerConfiguration *triggerConfig        /* [output] Returns default setting */
);

BERR_Code BI2C_Auto_P_ReadEDDC(
    void                *context,           /* Device channel handle */
    uint8_t             chipAddr,           /* chip address */
    uint8_t             segment,            /* EDDC segment pointer */
    uint32_t            subAddr,            /* 8-bit sub address */
    uint8_t             *pData,             /* pointer to memory location to store read data  */
    size_t              length              /* number of bytes to read */
    );

BERR_Code BI2C_Auto_P_Read
(
    void *context,                          /* Device channel handle */
    uint16_t chipAddr,                      /* chip address */
    uint32_t subAddr,                       /* 8-bit sub address */
    uint8_t *pData,                         /* pointer to memory location to store read data  */
    size_t length                           /* number of bytes to read */
    );

BERR_Code BI2C_Auto_P_ReadNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
);

BERR_Code BI2C_Auto_P_Write
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
);

BERR_Code BI2C_AUTO_P_ReadBy4BytesCmd
(
    BI2C_ChannelHandle  hChn,               /* Device channel handle */
    uint16_t            chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t            subAddr,            /* sub address */
    uint8_t             numSubAddrBytes,    /* number of bytes in register address */
    uint8_t             *pData,             /* storage */
    size_t              numBytes,           /* number of bytes to read */
    bool                mutex,              /* protect with a mutex */
    bool                ack,                /* check for ack? */
    bool                noStop              /* no stop at the end? */
);

BERR_Code BI2C_AUTO_P_WriteBy4BytesCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t            chipAddr,       /* i2c chip address.  this is unshifted */
    uint32_t            subAddr,        /* sub address */
    uint8_t             numSubAddrBytes,    /* number of bytes in register address */
    const uint8_t       *pData,         /* storage */
    size_t              numBytes,       /* number of bytes to write */
    bool                isNvram,        /* is this a nvram access? */
    bool                mutex,           /* protect with a mutex */
    bool                ack,            /* check for ack? */
    bool                noStop          /* do we need a stop at the end of the transfer */
);


#ifdef __cplusplus
}
#endif

#endif



