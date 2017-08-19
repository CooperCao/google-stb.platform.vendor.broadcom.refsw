/*
 * USB Device Controller API.
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

#ifndef _USBCONTROLLER_H_
#define _USBCONTROLLER_H_

#include <osl_decl.h>
#include <typedefs.h>
#include <urb.h>
#include <usbtypes.h>
#include <usbendpoint.h>

#define USBCONTROLLER_ATTRIBUTE_ISOC_BUFFER		0x01
#define USBCONTROLLER_ATTRIBUTE_RESUME_ENABLE		0x02
#define USBCONTROLLER_ATTRIBUTE_FORCE_WORD_ALIGN	0x04
#define USBCONTROLLER_ATTRIBUTE_TRAN_DETECT		0x08
#define USB_DIR_HOST_TO_DEVICE				0x00
#define USB_DIR_DEVICE_TO_HOST				0x80

struct _usb_device;
struct _usb_controller;

#define USB_PORT_1 0

typedef struct _usb_controller_function_table {
	usb_endpoint * (*ep_create)(struct _usb_controller *, endpoint_usage usage);
	void (*ep_destroy)(struct _usb_controller *, usb_endpoint *ep);
	void (*start)(struct _usb_controller *);
	void (*stop)(struct _usb_controller *);
	void (*enable)(struct _usb_controller *);
	void (*disable)(struct _usb_controller *);
	void (*reset)(struct _usb_controller *);
	void (*bus_reset)(struct _usb_controller *);
	void (*remote_wake)(struct _usb_controller *);
	bool (*update_resume)(struct _usb_controller *);
	void (*setup_complete)(struct _usb_controller *, bdc_status_code status);
	void (*interrupt_handler)(struct _usb_controller *);
	int (*get_port)(struct _usb_controller *);
	void (*data_sent)(struct _usb_controller *, usb_endpoint *);
	bool (*is_remote_wake_enabled)(struct _usb_controller *);
	void (*set_wakeup_source)(struct _usb_controller *);
	/*
	 * TODO: void (*pre_sleep_processing)(struct _usb_controller*, pmu_sleep_t sleep_state);
	 * TODO: void (*post_sleep_processing)(struct _usb_controller*, pmu_sleep_t sleep_state);
	 */
	void (*send_raw_event)(struct _usb_controller *, uint8 *data, uint32 length);
	void (*suspended)(struct _usb_controller *);
	void (*transfer_start)(struct _usb_controller *, urb_t *urb);
	void (*data_received)(struct _usb_controller *, usb_endpoint *);
	void (*transfer_complete)(struct _usb_controller *, usb_endpoint *, bdc_status_code);
	void (*ep_cancel_all_transfer)(struct _usb_controller *, usb_endpoint *);
	/* TODO: void (*add_sof_call_back)(struct _usb_controller*, Call_back_list* a_list); */
	void (*connect_to_host)(struct _usb_controller *);
	void (*disconnect_from_host)(struct _usb_controller *);
	bdc_status_code
		(*feature)(struct _usb_controller *, uint16 feature, int is_set);
} usb_controller_function_table;

typedef struct _usb_controller {
	struct _usb_device *device;
	/* Call_back_list* sof_call_back_list; */
	struct _urb_t *pending_urbs_list;
	struct _usb_controller_function_table *ft;

	osl_t *osh;
	uint16 attribute;
	uint8 max_port;
	uint8 reserved2;
} usb_controller;

/* void usbcontroller_add_sof_call_back(usb_controller* controller, Call_back_list* a_list); */
void usbcontroller_connect_to_host(usb_controller *controller);
void usbcontroller_disconnect_from_host(usb_controller *controller);
void usbcontroller_enable(usb_controller *controller);
void usbcontroller_disable(usb_controller *controller);
void usbcontroller_send_raw_event(usb_controller *controller, uint8 *data, uint32 length);
void usbcontroller_suspended(usb_controller *controller);
usb_endpoint * usbcontroller_ep_create(usb_controller *controller, endpoint_usage usage);
void usbcontroller_start(usb_controller *controller);
void usbcontroller_stop(usb_controller *controller);
void usbcontroller_setup_complete(usb_controller *controller, bdc_status_code status);
/*
 * void usbcontroller_pre_sleep_processing(usb_controller* controller, pmu_sleep_t sleep_state);
 * void usbcontroller_post_sleep_processing(usb_controller* controller, pmu_sleep_t sleep_state);
 */
void usbcontroller_transfer_start(usb_controller *controller, urb_t *urb);
void usbcontroller_data_received(usb_controller *controller, usb_endpoint *ep);
void usbcontroller_transfer_complete(usb_controller *controller, usb_endpoint *ep,
		bdc_status_code status);
void usbcontroller_ep_cancel_all_transfer(usb_controller *controller, usb_endpoint *ep);
bdc_status_code usbcontroller_feature(usb_controller *controller, uint16 feature, int is_set);
void usbcontroller_polling(usb_controller *controller);
int usbcontroller_get_port(usb_controller *controller);
void usbcontroller_set_wakeup_source(usb_controller *controller);
void usbcontroller_copy_to_fifo(uint8 *dest, uint8 *src, uint32 length);
void usbcontroller_copy_from_fifo(uint8 *dest, uint8 *src, uint32 length);
void usbcontroller_unalign_transfer(urb_t *urb, uint32 packet_size);
void usbcontroller_ep_destroy(usb_controller *controller, usb_endpoint *ep);
void usbcontroller_bus_reset(usb_controller *controller);
void usbcontroller_reset(usb_controller *controller);
void usbcontroller_data_sent(usb_controller *controller, usb_endpoint *ep);
bool usbcontroller_is_enumerated(usb_controller *);
void usbcontroller_resume_completed(usb_controller *controller);
bool usbcontroller_update_resume(usb_controller *controller);

void usbcontroller_init(usb_controller *controller);


/* These functions needs to be implemented by every controller */
#if !CONFIG_NO_TRANSPORT_DMA
extern void (*usbcontroller_handle_rx_dma)(void);
extern void (*usbcontroller_handle_tx_dma)(void);
#endif

extern void (*usbcontroller_hub_init)(usb_controller *hub);

#endif /* _USBCONTROLLER_H_ */
