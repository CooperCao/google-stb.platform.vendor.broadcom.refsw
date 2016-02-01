/***************************************************************************
*     Copyright (c) 2002-2014, Broadcom Corporation
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
BERR_Code BI2C_P_WriteSwCmd
(
    BI2C_ChannelHandle  hChn,            /* Device channel handle */
    uint16_t             chipAddr,       /* i2c chip address.  this is unshifted */
    uint32_t             subAddr,        /* pointer to register address */
    uint8_t          numSubAddrBytes,    /* number of bytes in register address */
    const uint8_t        *pData,         /* storage */
    size_t           numBytes,           /* number of bytes to write */
    bool                 isNvram         /* is this a nvram access? */
);


BERR_Code BI2C_P_WriteSw
(
    void *context,                       /* Device channel handle */
    uint16_t chipAddr,                   /* chip address */
    uint32_t subAddr,                    /* 8-bit sub address */
    const uint8_t *pData,                /* pointer to data to write */
    size_t length                        /* number of bytes to write */
);

BERR_Code BI2C_P_WriteSwNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    const uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
);

BERR_Code BI2C_P_WriteSwA16
(
    void *context,                      /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 16-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
);

BERR_Code BI2C_P_WriteSwA24
(
    void *context,                      /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 24-bit sub address */
    const uint8_t *pData,               /* pointer to data to write */
    size_t length                       /* number of bytes to write */
);

BERR_Code BI2C_P_ReadSwCmd
(
    BI2C_ChannelHandle  hChn,           /* Device channel handle */
    uint16_t        chipAddr,           /* i2c chip address.  this is unshifted */
    uint32_t        subAddr,            /* pointer to register address */
    uint8_t         numSubAddrBytes,    /* number of bytes in register address */
    uint8_t         *pData,             /* storage */
    size_t          numBytes,           /* number of bytes to read */
    bool            eddc,               /* EDDC mode */
    uint8_t         segment,            /* EDDC segment */
    bool            checkForAck         /* Check for ack? */
);

BERR_Code BI2C_P_ReadSw
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,          /* chip address */
    uint32_t subAddr,           /* 8-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length               /* number of bytes to read */
);

BERR_Code BI2C_P_ReadSwNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,          /* chip address */
    uint32_t subAddr,           /* 8-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length               /* number of bytes to read */
);

BERR_Code BI2C_P_ReadSwA16
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 16-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
);

BERR_Code BI2C_P_ReadSwA24
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint32_t subAddr,                   /* 24-bit sub address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
);

BERR_Code BI2C_P_ReadSwNoAddr
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
);

BERR_Code BI2C_P_ReadSwNoAddrNoAck
(
    void *context,              /* Device channel handle */
    uint16_t chipAddr,                  /* chip address */
    uint8_t *pData,             /* pointer to memory location to store read data  */
    size_t length                       /* number of bytes to read */
);

BERR_Code BI2C_P_ReadSwEDDC(
    void                *context,               /* Device channel handle */
    uint8_t             chipAddr,               /* chip address */
    uint8_t             segment,                /* EDDC segment */
    uint32_t             subAddr,                /* 8-bit sub address */
    uint8_t             *pData,                 /* pointer to memory location to store read data */
    size_t              length                  /* number of bytes to read */
);
