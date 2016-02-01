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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/****************************************************************************/
/*
 *  SCD internal
 */
/****************************************************************************/

#ifndef SCD_INT_H
#define SCD_INT_H

/****************************************************************************/
#ifdef BBS_SCD_PERL
#include "tunerTfe.h"
#include "tunerTfe_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* constants */
/****************************************************************************/

#define SEMAPHORE_NAME__CHIP   0

#define SCD_STATE_UNINITIALIZED  0
#define SCD_STATE_CLOSED         1
#define SCD_STATE_OPEN           2
#define SCD_STATE_RUNNING        3

#define SCD_HANDLE_INVALID 0
#define SCD_HANDLE_CHIP    1
#define SCD_HANDLE_FAT     2
#define SCD_HANDLE_FDC     3

/****************************************************************************/
/* macros */
/****************************************************************************/

#define GET_HANDLE_TYPE(handle)  (*((uint32_t *) handle)) /* (*(handle).p_id) */
#define IS_BAD_HANDLE(handle)  ((handle) == SCD_NULL)
#define IS_NOT_HANDLE_TYPE(handle, type)  (IS_BAD_HANDLE(handle) || (GET_HANDLE_TYPE(handle) != (type)))
#define IS_DEMOD_HANDLE(handle)  ((GET_HANDLE_TYPE(handle) == SCD_HANDLE_FAT) || (GET_HANDLE_TYPE(handle) == SCD_HANDLE_FDC))

/****************************************************************************/
/* types */
/****************************************************************************/

/* chip functions */
typedef SCD_RESULT (*SCD_CHIP_OPEN_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle);
typedef SCD_RESULT (*SCD_CHIP_CLOSE_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle);
typedef SCD_RESULT (*SCD_CHIP_START_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_MOD_FORMAT format);
typedef SCD_RESULT (*SCD_CHIP_STOP_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle);
typedef SCD_RESULT (*SCD_CHIP_SET_FREQUENCY_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle, uint32_t frequency, SCD_MOD_FORMAT format);
typedef SCD_RESULT (*SCD_CHIP_SET_CONFIG_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_CONFIG_ITEM item, void *buffer, uint32_t length);
typedef SCD_RESULT (*SCD_CHIP_GET_STATUS_FUNC)(SCD_HANDLE chip_handle, SCD_HANDLE handle, SCD_STATUS_ITEM item, void *buffer, uint32_t length);
typedef SCD_RESULT (*SCD_CHIP_WRITE_GPIO_FUNC)(SCD_HANDLE chip_handle, uint32_t addr, uint32_t value);
typedef SCD_RESULT (*SCD_CHIP_READ_GPIO_FUNC)(SCD_HANDLE chip_handle, uint32_t addr, uint32_t *value);
typedef SCD_RESULT (*SCD_CHIP_WRITE_I2C_FUNC)(SCD_HANDLE chip_handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer);
typedef SCD_RESULT (*SCD_CHIP_READ_I2C_FUNC)(SCD_HANDLE chip_handle, uint32_t i2c_addr, uint32_t i2c_subaddr, uint32_t subaddr_len, uint32_t data_len, uint8_t *buffer);
typedef SCD_RESULT (*SCD_CHIP_GET_VERSION_FUNC)(SCD_HANDLE chip_handle, SCD_ITEM item, SCD_VERSION *version);
typedef SCD_RESULT (*SCD_CHIP_WRITE_FUNC)(SCD_HANDLE chip_handle, uint32_t offset, uint32_t data_len, uint8_t *buffer);
typedef SCD_RESULT (*SCD_CHIP_READ_FUNC)(SCD_HANDLE chip_handle, uint32_t offset, uint32_t data_len, uint8_t *buffer);

typedef struct
{
    SCD_CHIP_OPEN_FUNC open;
    SCD_CHIP_CLOSE_FUNC close;
    SCD_CHIP_START_FUNC start;
    SCD_CHIP_STOP_FUNC stop;
    SCD_CHIP_SET_FREQUENCY_FUNC set_frequency;
    SCD_CHIP_SET_CONFIG_FUNC set_config;
    SCD_CHIP_GET_STATUS_FUNC get_status;
    SCD_CHIP_WRITE_GPIO_FUNC write_gpio;
    SCD_CHIP_READ_GPIO_FUNC read_gpio;
    SCD_CHIP_WRITE_I2C_FUNC write_i2c;
    SCD_CHIP_READ_I2C_FUNC read_i2c;
    SCD_CHIP_GET_VERSION_FUNC get_version;
    SCD_CHIP_WRITE_FUNC write;
    SCD_CHIP_READ_FUNC read;
} SCD_CHIP_FUNCTIONS;

/* objects */

typedef struct
{
    uint32_t handle_id;
    void *user;
    uint32_t open_count;
    uint32_t chip_instance;
    uint32_t state;
    SCD_CHIP_FUNCTIONS function;
    SCD_HANDLE mutex_handle;
} SCD_CHIP;

typedef struct
{
    uint32_t handle_id;
    uint32_t open_count;
    uint32_t demod_instance;
    uint32_t chip_instance;
    uint32_t state;
    uint32_t last_mod;
    const char *name;
} SCD_DEMOD;

typedef union
{
    uint32_t *p_id;
    SCD_CHIP *chip;
    SCD_DEMOD *demod;
    SCD_HANDLE handle;
} SCD_OBJECT;

/****************************************************************************/
/* prototypes */
/****************************************************************************/
SCD_RESULT BTFE_P_ScdAddChip(uint32_t chip_instance, SCD_CHIP_FUNCTIONS *functions, SCD_HANDLE *chip_handle, void *user);
SCD_RESULT BTFE_P_ScdAddFat(uint32_t chip_instance, uint32_t tuner_instance, const char *name, SCD_HANDLE *fat_handle);
SCD_RESULT BTFE_P_ScdGetInstance(SCD_HANDLE handle, uint32_t *instance);
SCD_RESULT BTFE_P_ScdGetChipFromInstance(uint32_t chip_instance, SCD_HANDLE *chip_handle);
SCD_RESULT BTFE_P_ScdGetUser(SCD_HANDLE handle, void **user);
SCD_RESULT BTFE_P_ScdGetRFtop(SCD_HANDLE handle, uint8_t *RFtopIndex, uint8_t *maxRFtopIndex, uint8_t *RFtopBackoff_dB, char *top_String);
SCD_RESULT BTFE_P_ScdSetRFtop(SCD_HANDLE handle, uint8_t RFtopIndex);

/****************************************************************************/

#ifdef __cplusplus
}
#endif

/****************************************************************************/

#endif /* SCD_INT_H */

/****************************************************************************/
