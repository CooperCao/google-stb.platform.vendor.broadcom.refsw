/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * [File Description:]
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
