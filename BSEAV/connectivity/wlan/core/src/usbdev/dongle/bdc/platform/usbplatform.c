/*
 *  USB Internal configuration
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <usbplatform.h>

usb_internal_config_t usb_internal_config =
{
/* isoc_buffer: */
	TRUE,

/* isoc_word_align_wa: */
#ifdef USB3_TRANSFER_WORD_ALIGN_WA
	TRUE,
#else
	FALSE,
#endif

/* l1rwe_status_wa: */
#ifdef USB_BDC_RWE_STATUS_WA
	TRUE,
#else
	FALSE,
#endif

/* l1rwe_enable_wa: */
#ifdef USB_BDC_RWE_ENABLE_WA
	TRUE,
#else
	FALSE,
#endif

/* pending_dma_at_bus_reset_wa: */
	FALSE,

/* default_save_restore_en: */
#ifdef USB_BDC_DEFAULT_SAVE_RESTORE_EN
	TRUE,
#else
	FALSE,
#endif

/* bus_reset_hw_reset: */
	FALSE,

/* usused1 */
	0,
	};
