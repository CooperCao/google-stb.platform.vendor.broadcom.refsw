/*
 * Copyright (C) 2018 Broadcom
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

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include <linux/spi/spi.h>
#include "spi_driver.h"

#define DRIVER_NAME	    "nexus_spi_shim"
#define MAJOR_DEVICE    35 /* Major device number */
#define N_SPI_MINORS	32	/* ... up to 256 */

static DECLARE_BITMAP(minors, N_SPI_MINORS);

#define BUF_SIZE            4096

struct spidrv_info {
    dev_t              devt;
    spinlock_t         spi_lock;
    struct mutex       buf_lock;
    struct spi_device  *spi;
    u8			       *tx_buffer;
	u8			       *rx_buffer;
    u8                 open;
    struct spidrv_settings settings;
};

struct spidrv_info *spidrv[UPG_MSPI_MAX_CS] = {0};

static DEFINE_MUTEX(device_list_lock);

int gSpiDebug=0;
module_param(gSpiDebug,int,0000);

int num_spi_devices=0;
EXPORT_SYMBOL(num_spi_devices);

static int spidrv_sync(struct spidrv_info *spi_driver, struct spi_message *message)
{
    int status;
    struct spi_device *spi;

    spin_lock_irq(&spi_driver->spi_lock);
    spi = spi_driver->spi;
    spin_unlock_irq(&spi_driver->spi_lock);

    if (spi_driver->spi == NULL)
        status = -ESHUTDOWN;
    else
        status = spi_sync(spi, message);

    return status;
}

static int spidrv_message_multiple (struct spidrv_info *spi_driver, struct spidrv_message_multiple *msgs)
{
    struct spi_message	m;
    struct spi_transfer	*t;
    struct spidrv_message *msg;
    uint32_t i, size = 0, buf_size = 0;
    u8 *data_buf, *buf;
    int status = 0;

    spi_message_init(&m);

    /* Currently only handling write messages */

    size = msgs->count*sizeof(struct spidrv_message);
    msg = kmalloc(size, GFP_KERNEL);
    if (!msg) {
        status = -ENOMEM;
        goto err;
    }
    /* Must be continuous address */
    if (copy_from_user(msg, msgs->msg, size)) {
            status = -EFAULT;
            goto err_msg;
    }

    for(i=0; i< msgs->count; i++) {
        buf_size += msg[i].len;
    }
    data_buf = kmalloc(buf_size, GFP_KERNEL);
    if (!data_buf) {
        status = -ENOMEM;
        goto err_msg;
    }

    size = msgs->count*sizeof(struct spi_transfer);
    t = kzalloc(size, GFP_KERNEL);
    if (!t) {
        status = -ENOMEM;
        goto err_data;
    }

    buf = data_buf;

    for(i=0; i< msgs->count; i++) {
        t[i].tx_buf = buf;
        if (copy_from_user(buf, msg[i].tx_buf, msg[i].len)) {
            status = -EFAULT;
            goto err_t;
        }
        buf += msg[i].len;
        t[i].len = msg[i].len;
        t[i].speed_hz = spi_driver->settings.speed;

        spi_message_add_tail(&t[i], &m);
    }

    status =  spidrv_sync(spi_driver, &m);

err_t:
    kfree(t);
err_data:
    kfree(data_buf);
err_msg:
    kfree(msg);
err:
    return status;
}

int spidrv_driver_message_multiple(void *context, struct spidrv_message_multiple *msgs)
{
    struct spidrv_info *spi_driver = (struct spidrv_info *) context;
    struct spi_message	m;
    struct spi_transfer	*t;
    struct spidrv_message *msg = msgs->msg;
    uint32_t i, size;
    int status = 0;

    mutex_lock(&spi_driver->buf_lock);

    spi_message_init(&m);

    /* Currently only handling write messages */

    size = msgs->count*sizeof(struct spi_transfer);
    t = kzalloc(size, GFP_KERNEL);

    for(i=0; i< msgs->count; i++) {
        t[i].tx_buf = msg[i].tx_buf;
        t[i].len = msg[i].len;
        t[i].speed_hz = spi_driver->settings.speed;

        spi_message_add_tail(&t[i], &m);
    }

    status =  spidrv_sync(spi_driver, &m);

    kfree(t);

    mutex_unlock(&spi_driver->buf_lock);

    return status;
}
EXPORT_SYMBOL(spidrv_driver_message_multiple);

static int spidrv_message(struct spidrv_info *spi_driver, struct spidrv_message *msg)
{
    struct spi_message	m;
    struct spi_transfer	t={0};
    int status = 0;

    spi_message_init(&m);

    if (msg->tx_buf) {
        if (copy_from_user(spi_driver->tx_buffer, msg->tx_buf, msg->len)) {
            status = -EFAULT;
            goto err;
        }
        t.tx_buf = spi_driver->tx_buffer;
    }
    if (msg->rx_buf) {
        t.rx_buf = spi_driver->rx_buffer;
    }
    t.len = msg->len;
    t.speed_hz = spi_driver->settings.speed;

    spi_message_add_tail(&t, &m);

    status =  spidrv_sync(spi_driver, &m);
    if (status !=0)
        goto err;

    if (msg->rx_buf) {
        if (copy_to_user(msg->rx_buf, spi_driver->rx_buffer, msg->len)) {
            status = -EFAULT;
            goto err;
        }
    }

err:
    return status;
}

int spidrv_driver_message(void *context, struct spidrv_message *msg)
{
    struct spidrv_info *spi_driver = (struct spidrv_info *) context;
    struct spi_message	m;
    struct spi_transfer	t={0};
    int status = 0;

    mutex_lock(&spi_driver->buf_lock);

    spi_message_init(&m);

    t.tx_buf = msg->tx_buf;
    t.rx_buf = msg->rx_buf;
    t.len = msg->len;
    t.speed_hz = spi_driver->settings.speed;

    spi_message_add_tail(&t, &m);

    status =  spidrv_sync(spi_driver, &m);

    mutex_unlock(&spi_driver->buf_lock);

    return status;
}
EXPORT_SYMBOL(spidrv_driver_message);

int spidrv_driver_get_settings(void *context, struct spidrv_settings *settings)
{
    struct spidrv_info *spi_driver = (struct spidrv_info *) context;
    *settings = spi_driver->settings;
    return 0;
}
EXPORT_SYMBOL(spidrv_driver_get_settings);

int spidrv_p_set_settings(struct spidrv_info *spi_driver, struct spi_device	*spi, struct spidrv_settings *settings)
{
    int result = 0;

    if (spi_driver->settings.speed != settings->speed) {
        int retval = 0;
        u32	save = spi->max_speed_hz;
        spi->max_speed_hz = settings->speed;
        retval = spi_setup(spi);
        spi->max_speed_hz = save;
        if (retval < 0) {
            pr_err("%d Hz (max)\n", settings->speed);
            result = -EPERM;
            goto err;
        } else {
            spi_driver->settings.speed = settings->speed;
        }
    }
    if (spi_driver->settings.bits_per_word != settings->bits_per_word) {
        int retval = 0;
        u8	save = spi->bits_per_word;
        spi->bits_per_word = settings->bits_per_word;
        retval = spi_setup(spi);
        if (retval < 0) {
            spi->bits_per_word = save;
            result = -EPERM;
            goto err;
        } else {
            spi_driver->settings.bits_per_word = settings->bits_per_word;
        }
    }
    if (spi_driver->settings.clock_active_low != settings->clock_active_low ||
        spi_driver->settings.tx_leading_cap_falling != settings->tx_leading_cap_falling) {
        int retval = 0;
        u32	save = spi->mode;

        if (settings->clock_active_low)
            spi->mode |= SPI_CPOL;
        else
            spi->mode &= ~SPI_CPOL;
        if (settings->tx_leading_cap_falling)
            spi->mode |= SPI_CPHA;
        else
            spi->mode &= ~SPI_CPHA;

        retval = spi_setup(spi);
        if (retval < 0) {
            result = -EPERM;
            spi->mode = save;
            goto err;
        } else {
            spi_driver->settings.clock_active_low = settings->clock_active_low;
            spi_driver->settings.tx_leading_cap_falling = settings->tx_leading_cap_falling;
        }
    }

err:
    return result;
}

int spidrv_driver_set_settings(void *context, struct spidrv_settings *settings)
{
    struct spidrv_info *spi_driver = (struct spidrv_info *) context;
    struct spi_device	*spi;
    int status;

    spin_lock_irq(&spi_driver->spi_lock);
	spi = spi_dev_get(spi_driver->spi);
    spin_unlock_irq(&spi_driver->spi_lock);

    mutex_unlock(&spi_driver->buf_lock);
    status = spidrv_p_set_settings(spi_driver, spi, settings);
    mutex_unlock(&spi_driver->buf_lock);

    spi_dev_put(spi);

    return status;
}
EXPORT_SYMBOL(spidrv_driver_set_settings);

static long spidrv_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
    struct spidrv_info *spi_driver = file->private_data;
    struct spi_device	*spi;
    int result = 0;

    spin_lock_irq(&spi_driver->spi_lock);
	spi = spi_dev_get(spi_driver->spi);
    spin_unlock_irq(&spi_driver->spi_lock);

    if (spi == NULL)
		return -ESHUTDOWN;

    mutex_lock(&spi_driver->buf_lock);

    switch (cmd) {
        case BRCM_IOCTL_SPI_GET:
            if(copy_to_user((void*)arg, &spi_driver->settings, sizeof(struct spidrv_settings))) {
                result = -EFAULT;
                pr_err("copy_to_user failed!\n");
                break;
            }
            break;
        case BRCM_IOCTL_SPI_SET:
            {
                struct spidrv_settings settings;
                if (copy_from_user(&settings, (void*)arg, sizeof(settings))) {
                    result = -EFAULT;
                    pr_err("copy_from_user failed!\n");
                    break;
                }
                result = spidrv_p_set_settings(spi_driver, spi, &settings);
            }
            break;
        case BRCM_IOCTL_SPI_MSG:
            {
                struct spidrv_message msg;
                if (copy_from_user(&msg, (void*)arg, sizeof(msg))) {
                    result = -EFAULT;
                    pr_err("copy_from_user failed!\n");
                    break;
                }
                if (msg.len > BUF_SIZE) {
                    result = -EMSGSIZE;
                    pr_err("Message length exceeded buffer size!\n");
                    break;
                }
                result = spidrv_message(spi_driver, &msg);
            }
            break;
        case BRCM_IOCTL_SPI_MSG_MULT:
            {
                struct spidrv_message_multiple msgs;
                if (copy_from_user(&msgs, (void*)arg, sizeof(msgs))) {
                    result = -EFAULT;
                    pr_err("copy_from_user failed!\n");
                    break;
                }
                result = spidrv_message_multiple(spi_driver, &msgs);
            }
            break;
        default:
            pr_err("Unknown Ioctl\n");
            break;
    }

    mutex_unlock(&spi_driver->buf_lock);

    spi_dev_put(spi);

    return result;
}

static long  spidrv_compat_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
    return spidrv_ioctl(file, cmd, arg);
}

static int spidrv_p_open(struct spidrv_info *spi_driver)
{
    int	status = 0;

    if (spi_driver->open) {
        pr_err("spi_driver: already open %d\n", spi_driver->spi->chip_select);
        status = -EBUSY;
        goto err;
    }

    if (!spi_driver->tx_buffer) {
		spi_driver->tx_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
		if (!spi_driver->tx_buffer) {
            pr_err("open/ENOMEM\n");
            status = -ENOMEM;
            goto err;
        }
    }

	if (!spi_driver->rx_buffer) {
		spi_driver->rx_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
		if (!spi_driver->rx_buffer) {
			pr_err("open/ENOMEM\n");
			status = -ENOMEM;
			goto err_alloc;
		}
	}

    spi_driver->open = true;

    return status;

err_alloc:
	kfree(spi_driver->tx_buffer);
	spi_driver->tx_buffer = NULL;
err:
    return status;
}

void *spidrv_driver_open(uint32_t index)
{
    struct spidrv_info *spi_driver = NULL;
    int	status;

    mutex_lock(&device_list_lock);

    if (index > UPG_MSPI_MAX_CS) {
        pr_err("spi_driver: Cannot open index %d\n", index);
        goto err;
    }

    spi_driver = spidrv[index];

    status = spidrv_p_open(spi_driver);
    if (status) {
        spi_driver = NULL;
    }

    mutex_unlock(&device_list_lock);

err:
    return (void*)spi_driver;
}
EXPORT_SYMBOL(spidrv_driver_open);

static int spidrv_open(struct inode *inode, struct file *file)
{
    struct spidrv_info *spi_driver;
    int	status = 0;
    int i;

    mutex_lock(&device_list_lock);

    for (i=0; i<UPG_MSPI_MAX_CS; i++) {
        spi_driver = spidrv[i];
        if (spi_driver && (spi_driver->devt == inode->i_rdev)) {
			break;
		}
    }

    if (i == UPG_MSPI_MAX_CS) {
        pr_err("spi_driver: nothing for minor %d\n", iminor(inode));
        status = -ENXIO;
        goto err;
    }

    status = spidrv_p_open(spi_driver);
    if (status) {
        goto err;
    }

    file->private_data = spi_driver;
    nonseekable_open(inode, file);

err:
	mutex_unlock(&device_list_lock);
	return status;
}

static void spidrv_p_close(struct spidrv_info *spi_driver)
{
    spi_driver->open = false;

    if (spi_driver->tx_buffer) {
        kfree(spi_driver->tx_buffer);
        spi_driver->tx_buffer = NULL;
    }

    if (spi_driver->rx_buffer) {
        kfree(spi_driver->rx_buffer);
        spi_driver->rx_buffer = NULL;
    }

    return;
}

void spidrv_driver_close(void *context)
{
    struct spidrv_info *spi_driver;

    mutex_lock(&device_list_lock);
    spi_driver = (struct spidrv_info *) context;
    spidrv_p_close(spi_driver);
    mutex_unlock(&device_list_lock);

    return;
}
EXPORT_SYMBOL(spidrv_driver_close);

static int spidrv_close(struct inode *inode, struct file *file)
{
    struct spidrv_info *spi_driver;

    mutex_lock(&device_list_lock);
	spi_driver = file->private_data;
	file->private_data = NULL;

    spidrv_p_close(spi_driver);

    mutex_unlock(&device_list_lock);

    return 0;
}

static const struct file_operations spi_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = spidrv_ioctl,
    .compat_ioctl = spidrv_compat_ioctl,
    .open = spidrv_open,
    .release = spidrv_close,
};

static struct class *spidrv_class;

static int spidrv_probe(struct spi_device *spi)
{
    struct spidrv_info *spi_driver;
    int			status = 0;
	unsigned long		minor;

    if (gSpiDebug)
        pr_info("spidrv_probe : spi_device chip select %d\n", spi->chip_select);

    if (spi->chip_select >= UPG_MSPI_MAX_CS) {
        pr_err("Chip Select %d not supported (%d)", spi->chip_select, UPG_MSPI_MAX_CS);
        return -ENXIO;
    }

    /* Allocate driver info */
	spi_driver = kzalloc(sizeof(*spi_driver), GFP_KERNEL);
	if (!spi_driver)
		return -ENOMEM;

	/* Initialize the driver data */
	spi_driver->spi = spi;
    spin_lock_init(&spi_driver->spi_lock);
    mutex_init(&spi_driver->buf_lock);

    mutex_lock(&device_list_lock);
	minor = find_first_zero_bit(minors, N_SPI_MINORS);
	if (minor < N_SPI_MINORS) {
		struct device *dev;

		spi_driver->devt = MKDEV(MAJOR_DEVICE, minor);
		dev = device_create(spidrv_class, &spi->dev, spi_driver->devt,
				    spi_driver, "spi_drv%d", spi->chip_select);
        if (IS_ERR(dev)) {
            status = PTR_ERR(dev);
        }
	} else {
		pr_err("No minor number available!\n");
		status = -ENODEV;
	}
	if (status == 0) {
		set_bit(minor, minors);
	}
	mutex_unlock(&device_list_lock);

    spi_driver->settings.speed = spi->max_speed_hz;
    spi_driver->settings.bits_per_word = spi->bits_per_word;
    spi_driver->settings.clock_active_low = spi->mode&SPI_CPOL?true:false;
    spi_driver->settings.tx_leading_cap_falling = spi->mode&SPI_CPHA?true:false;

    if (status == 0)
        spi_set_drvdata(spi, spi_driver);
    else
		kfree(spi_driver);

    spidrv[spi->chip_select] = spi_driver;
    num_spi_devices++;

    return status;
}

static int spidrv_remove(struct spi_device *spi)
{
    struct spidrv_info *spi_driver = spi_get_drvdata(spi);

    if (gSpiDebug)
        pr_info("spidrv_remove : spi_device chip select %d\n", spi->chip_select);

    spin_lock_irq(&spi_driver->spi_lock);
    spi_driver->spi = NULL;
    spin_unlock_irq(&spi_driver->spi_lock);

    mutex_lock(&device_list_lock);
    device_destroy(spidrv_class, spi_driver->devt);
	clear_bit(MINOR(spi_driver->devt), minors);
    kfree(spi_driver);
    mutex_unlock(&device_list_lock);

    return 0;
}

static struct spi_driver spi_drv = {
	.driver = {
		.name =		DRIVER_NAME,
		.owner =	THIS_MODULE,
	},
	.probe =	spidrv_probe,
	.remove =	spidrv_remove,
};

static int __init spidrv_init(void)
{
    int status;

    pr_info("Initializing spi driver\n");

    status = register_chrdev(MAJOR_DEVICE, DRIVER_NAME, &spi_fops);
    if (status < 0)
    {
        pr_err("Unable to get major %d\n", MAJOR_DEVICE);
        return status;
    }

    spidrv_class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(spidrv_class)) {
		unregister_chrdev(MAJOR_DEVICE, DRIVER_NAME);
		return PTR_ERR(spidrv_class);
	}

    status = spi_register_driver(&spi_drv);
	if (status < 0) {
        class_destroy(spidrv_class);
		unregister_chrdev(MAJOR_DEVICE, DRIVER_NAME);
	}

    pr_info("Initialization complete\n");

	return status;
}
module_init(spidrv_init);

static void __exit spidrv_exit(void)
{
    pr_info("Cleanup spi driver\n");

    spi_unregister_driver(&spi_drv);
    class_destroy(spidrv_class);
    unregister_chrdev(MAJOR_DEVICE, DRIVER_NAME);

    pr_info("Cleanup complete\n");
}
module_exit(spidrv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Spi driver shim for Nexus");
