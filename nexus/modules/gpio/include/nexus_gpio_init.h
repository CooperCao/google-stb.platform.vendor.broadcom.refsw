/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NEXUS_GPIO_INIT_H__
#define NEXUS_GPIO_INIT_H__

/*=========================================
The Gpio module provides an Interface to set the value of GPIO output pins
and monitor any change in GPIO input pins.

It provides a single interface of the same name. See NEXUS_Gpio_Open.
===========================================*/

#include "nexus_types.h"
#include "nexus_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Number of GPIO banks supported by nexus

Description:
Each bank carries up to 32 gpio pins.  Not all chips have all banks populated.

**/
#define NEXUS_GPIO_NUM_BANKS 8

/**
Summary:
Gpio module capabilities

Description:

**/
typedef struct NEXUS_GpioModuleOsSharedBankCapabilities
{
    bool osManaged; /* whether the OS manages gpio programming in order to share register banks with the OS */
} NEXUS_GpioModuleOsSharedBankCapabilities;

/**
Summary:
Information required to set up the os sharing of gpio banks

Description:

**/
typedef struct NEXUS_GpioModuleOsSharedBankInitSettings
{
    uint32_t bankBaseAddresses[NEXUS_GPIO_NUM_BANKS];
    bool aon[NEXUS_GPIO_NUM_BANKS];
} NEXUS_GpioModuleOsSharedBankInitSettings;

/**
Summary:
Settings used to open a gpio pin that is in an os-shared bank

Description:

**/
typedef struct NEXUS_GpioModuleOsSharedBankPinOpenSettings
{
    uint32_t bankBaseAddress; /* base address of the bank this pin belongs to */
    unsigned shift; /* shift within the bank of this pin */
    NEXUS_GpioInterrupt interruptType; /* type of interrupt this pin uses */
} NEXUS_GpioModuleOsSharedBankPinOpenSettings;

/**
Summary:
Interrupt status of the os shared gpio banks

Description:

**/
typedef struct NEXUS_GpioModuleOsSharedBankInterruptStatus
{
    uint32_t status[NEXUS_GPIO_NUM_BANKS]; /* per-bank status */
} NEXUS_GpioModuleOsSharedBankInterruptStatus;

/**
Summary:
Handle for a given pin from an os-shared gpio bank

Description:

**/
typedef struct NEXUS_GpioModuleOsSharedBankPin * NEXUS_GpioModuleOsSharedBankPinHandle;

/**
Summary:
Settings used to configure the Gpio module for interaction with gpio
platform code required for sharing gpio banks with the OS.

Description:

**/
typedef struct
{
    NEXUS_Error (*init)(const NEXUS_GpioModuleOsSharedBankInitSettings * pSettings); /* initializes os gpio bank sharing */
    void (*getCapabilities)(NEXUS_GpioModuleOsSharedBankCapabilities * caps); /* returns the capabilities of the os gpio bank sharing module */
    NEXUS_Error (*getInterruptStatus_isr)(NEXUS_GpioModuleOsSharedBankInterruptStatus * pStatus); /* returns the interrupt status of all os-shared gpio banks */
    NEXUS_GpioModuleOsSharedBankPinHandle (*openPin)(const NEXUS_GpioModuleOsSharedBankPinOpenSettings * pSettings); /* opens a pin in an os-shared gpio bank, this implicitly tells the OS that nexus owns this pin */
    void (*closePin)(NEXUS_GpioModuleOsSharedBankPinHandle pin); /* closes a pin in an os-shared gpio bank, but does not release the pin from nexus ownership. Release of nexus ownership occurs once the driver is removed */
    NEXUS_Error (*clearPinInterrupt_isr)(NEXUS_GpioModuleOsSharedBankPinHandle pin); /* clears the interrupt status for a given pin in an os-shared gpio bank */
    NEXUS_Error (*setPinInterruptMask_isr)(NEXUS_GpioModuleOsSharedBankPinHandle pin, bool disable); /* enables/disables the interrupt mask for a given pin in an os-shared gpio bank */
    NEXUS_Error (*setStandby)(NEXUS_GpioModuleOsSharedBankPinHandle pin, bool enable); /* enables/disables standby mode for a given pin in an os-shared gpio bank. If the pin is an input with its interrupt enabled, it will act as a wakeup source */
} NEXUS_GpioModuleOsSharedBankSettings;

/**
Summary:
Settings used to configure the Gpio module.

Description:

See Also:
NEXUS_GpioModule_GetDefaultSettings
NEXUS_GpioModule_Init
**/
typedef struct NEXUS_GpioModuleSettings
{
    NEXUS_CommonModuleSettings common;
    NEXUS_GpioModuleOsSharedBankSettings osSharedBankSettings;
} NEXUS_GpioModuleSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.

See Also:
NEXUS_GpioModule_Init
**/
void NEXUS_GpioModule_GetDefaultSettings(
    NEXUS_GpioModuleSettings *pSettings /* [out] */
    );

/**
Summary:
Initialize the Gpio module.

Description:
This is called by the NEXUS Platform when the system is initialized.

See Also:
NEXUS_GpioModule_Uninit
NEXUS_Gpio_Open - open Interface for Gpio
**/
NEXUS_ModuleHandle NEXUS_GpioModule_Init(
    const NEXUS_GpioModuleSettings *pSettings
    );

/**
Summary:
Uninitialize the Gpio module.

Description:
This is called by the NEXUS Platform when the system is uninitialized.

See Also:
NEXUS_GpioModule_Init
**/
void NEXUS_GpioModule_Uninit(void);

#ifdef __cplusplus
}
#endif

#endif
