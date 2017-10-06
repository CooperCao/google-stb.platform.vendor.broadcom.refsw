/*
 * External driver API to ftdi_sio_brcm driver.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

typedef struct usb_serial_port * gpio_handle;

#define BITMODE_RESET 0x00
#define BITMODE_BITBANG 0x01

int ftdi_usb_reset(int handle);
int ftdi_set_bitmode(int handle, unsigned char bitmask, unsigned char mode);
int gpio_write_port(int handle, unsigned char pins);
int gpio_write_port_non_block(int handle, unsigned char pins);
int gpio_read_port(int handle, unsigned char *pins);
int handle_add(gpio_handle pointer);
int handle_remove(gpio_handle pointer);
int get_handle(const char *dev_filename);
gpio_handle get_pointer_by_handle(int handle);
