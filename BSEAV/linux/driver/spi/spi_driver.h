/*
 * Copyright (C) 2017 Broadcom
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, you may obtain a copy at
 * http://www.broadcom.com/licenses/LGPLv2.1.php or by writing to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef __SPI_DRIVER_H__
#define __SPI_DRIVER_H__
#include <linux/ioctl.h>

#define UPG_MSPI_MAX_CS        4

struct spidrv_settings {
    uint32_t speed;
    uint8_t bits_per_word;
    uint8_t clock_active_low;
    uint8_t tx_leading_cap_falling;
    /* TODO : dtl, rdsclk */
};

struct spidrv_message {
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    uint32_t len;
};

struct spidrv_message_multiple {
    struct spidrv_message *msg;
    uint32_t count;
};

#define BRCM_IOCTL_SPI_GET            _IOR(302, 1, struct spidrv_settings)
#define BRCM_IOCTL_SPI_SET            _IOW(302, 2, struct spidrv_settings)
#define BRCM_IOCTL_SPI_MSG            _IOW(302, 3, struct spidrv_message)
#define BRCM_IOCTL_SPI_MSG_MULT       _IOW(302, 4, struct spidrv_message_multiple)

void *spidrv_driver_open(uint32_t index);
void spidrv_driver_close(void *context);
int spidrv_driver_get_settings(void *context, struct spidrv_settings *settings);
int spidrv_driver_set_settings(void *context, struct spidrv_settings *settings);
int spidrv_driver_message_multiple(void *context, struct spidrv_message_multiple *msgs);
int spidrv_driver_message(void *context, struct spidrv_message *msg);

#endif /* __SPI_DRIVER_H__ */
