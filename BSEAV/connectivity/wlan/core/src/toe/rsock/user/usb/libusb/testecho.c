/*
 * testecho.c (created from testlibusb.c)
 *
 *  Send packets to a dongle that is set up to echo packets back.
 *  Don't ask how to make such a dongle...
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>
#include <stdlib.h> 					/* required for randomize() and random() */
#include <usb.h>

#define TRUE			1
#define FALSE			0

#define PKTSIZE			500			/* default packet size in [bytes] */
#define MAX_PKTSIZE		5000
#define PKTCOUNT		100

#define DONGLE_VENDOR		0x0a5c
#define DONGLE_PRODUCT_CDC	0x0cdc
#define DONGLE_PRODUCT_BDC	0x0bdc /* wifi usb dongle */

#define CDC_EP_DATA_IN		0x82
#define CDC_EP_DATA_OUT		0x03
#define BDC_EP_DATA_IN		0x82
#define BDC_EP_DATA_OUT		0x04

int verbose = 0;
volatile int pkt_size;
volatile int pkt_cnt;

volatile static int ep_data_in;			/* endpoint used for dongle->host traffic */
volatile static int ep_data_out;		/* endpoint used for host->dongle traffic */
volatile static int g_random_pkt_size = FALSE;  /* 1 == use random packet size */

double double_time(void)
{
    struct timeval	tv;

    gettimeofday(&tv, 0);

    return (tv.tv_sec + tv.tv_usec * 0.000001);
}

void print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
    printf("      bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
    printf("      bmAttributes:     %02xh\n", endpoint->bmAttributes);
    printf("      wMaxPacketSize:   %d\n", endpoint->wMaxPacketSize);
    printf("      bInterval:        %d\n", endpoint->bInterval);
    printf("      bRefresh:         %d\n", endpoint->bRefresh);
    printf("      bSynchAddress:    %d\n", endpoint->bSynchAddress);
}

void print_altsetting(struct usb_interface_descriptor *interface)
{
    int i;

    printf("    bInterfaceNumber:   %d\n", interface->bInterfaceNumber);
    printf("    bAlternateSetting:  %d\n", interface->bAlternateSetting);
    printf("    bNumEndpoints:      %d\n", interface->bNumEndpoints);
    printf("    bInterfaceClass:    %d\n", interface->bInterfaceClass);
    printf("    bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
    printf("    bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
    printf("    iInterface:         %d\n", interface->iInterface);

    for (i = 0; i < interface->bNumEndpoints; i++)
	print_endpoint(&interface->endpoint[i]);
}

void print_interface(struct usb_interface *interface)
{
    int i;

    for (i = 0; i < interface->num_altsetting; i++)
	print_altsetting(&interface->altsetting[i]);
}

void print_configuration(struct usb_config_descriptor *config)
{
    int i;

    printf("  wTotalLength:         %d\n", config->wTotalLength);
    printf("  bNumInterfaces:       %d\n", config->bNumInterfaces);
    printf("  bConfigurationValue:  %d\n", config->bConfigurationValue);
    printf("  iConfiguration:       %d\n", config->iConfiguration);
    printf("  bmAttributes:         %02xh\n", config->bmAttributes);
    printf("  MaxPower:             %d\n", config->MaxPower);

    for (i = 0; i < config->bNumInterfaces; i++)
	print_interface(&config->interface[i]);
}

int print_device(struct usb_device *dev, int level)
{
    usb_dev_handle *udev;
    char description[256];
    char string[256];
    int ret, i;

    udev = usb_open(dev);
    if (udev) {
	if (dev->descriptor.iManufacturer) {
	    ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
	    if (ret > 0)
		snprintf(description, sizeof(description), "%s - ", string);
	    else
		snprintf(description, sizeof(description), "%04X - ",
		         dev->descriptor.idVendor);
	} else
	    snprintf(description, sizeof(description), "%04X - ",
	             dev->descriptor.idVendor);

	if (dev->descriptor.iProduct) {
	    ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
	    if (ret > 0)
		snprintf(description + strlen(description), sizeof(description) -
		         strlen(description), "%s", string);
	    else
		snprintf(description + strlen(description), sizeof(description) -
		         strlen(description), "%04X", dev->descriptor.idProduct);
	} else
	    snprintf(description + strlen(description), sizeof(description) -
	             strlen(description), "%04X", dev->descriptor.idProduct);

    } else
	snprintf(description, sizeof(description), "%04X - %04X",
	         dev->descriptor.idVendor, dev->descriptor.idProduct);

    printf("%.*sDev #%d: %s\n", level * 2, "                    ", dev->devnum,
           description);

    if (udev && verbose) {
	if (dev->descriptor.iSerialNumber) {
	    ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string));
	    if (ret > 0)
		printf("%.*s  - Serial Number: %s\n", level * 2,
		       "                    ", string);
	}
    }

    if (udev)
	usb_close(udev);

    if (verbose) {
	if (!dev->config) {
	    printf("  Couldn't retrieve descriptors\n");
	    return 0;
	}

	for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
	    print_configuration(&dev->config[i]);
    } else {
	for (i = 0; i < dev->num_children; i++)
	    print_device(dev->children[i], level + 1);
    }

    return 0;
}

static volatile int rt_exit;
static volatile int rt_exited;
static volatile int rt_count;

static void *
reply_thread(void *arg)
{
    usb_dev_handle *udev = (usb_dev_handle *)arg;
    char *read_buf = NULL;
    int n;
    int i, cnt1=0, cnt2=0;
    int n_pkt_bytes; 		/* length of packet in [bytes] as indicated in packet header */

    //printf("reply_thread: entered\n");
    read_buf = (char*)malloc(pkt_size);
    if (!read_buf) {
	printf("Unable to allocate recv buffer, recv thread quitting!\n");
	goto end;
    }

    while (!rt_exit && (rt_count < pkt_cnt)) {
	// printf("reply_thread: bulk read len=%d\n", (int)sizeof(read_buf));

	if ((n = usb_bulk_read(udev, ep_data_in,
	                       read_buf, pkt_size,
	                      2000 /*timeout in msec*/)) < 0) {
	    fprintf(stderr,
	            "usb_bulk_read failed: errno=%d (%s). Expected to receive seqcnt=%d. Received total of %d packets, needed to receive %d.\n",
	            errno, strerror(errno), cnt1, rt_count, pkt_cnt);
	    goto end;
	}
	usleep(1);
//	recv_cnt++;
	bcopy(read_buf, &cnt2, 4);
	if(cnt1 == cnt2) {
		cnt1++;
	} else {
		printf("\nout of sequence! expected %d, received %d\n", cnt1, cnt2); 
		break;
		cnt1 = cnt2 + 1;
	}

//	printf("rx len %d:", n);
	bcopy(read_buf + 4, &n_pkt_bytes, 4);
	if(n != n_pkt_bytes) {
		printf("error: length in packet header (%d) differs from rx len\n", n_pkt_bytes);
	}
	/*
	 * Warning: printing the whole packet can lead to test errors because of the additional
	 * latency this introduces. So only comment out the lines below if you know what you are
	 * doing.
	 */
//	for (i = 0; i < n; i++)
//	    printf(" %02x", read_buf[i]);
//	printf("\n");

	rt_count++;
    }
end:
    if (read_buf)
	free(read_buf);

    //    printf("reply_thread: exiting\n");
    while (!rt_exit)
	usleep(10000);
    rt_exited = 1;
    pthread_exit(NULL);
}

/* generates a random number: rnd_min <= r <= rnd_max */
int rnd(int rnd_min, int rnd_max) {
	return rnd_min + 1.0 * rand() / RAND_MAX * (rnd_max - rnd_min);
}

int
main(int argc, char *argv[])
{
    struct usb_bus *bus;
    struct usb_device *dev = NULL;
    usb_dev_handle *udev;
    char *buf = NULL;
    int i, j, rv, desc_size = 512, seq_size = 4;
    pthread_t dt;
    double stime, etime;

    printf("Usage:   testecho [[pkt_count(100)] [pkt_size(500) or 'r' for 'random']]\n");
    printf("Example: testecho 400 r	: transmit 400 random sized packets\n");
    printf("Example: testecho 400 r12	: transmit 400 random sized packets with a seed of 12\n");
	pkt_size = PKTSIZE; 
	pkt_cnt  = PKTCOUNT;
    
    if (argc > 1) {
	pkt_cnt = atoi(argv[1]);
    }
    if (argc > 2) {
	char *p= argv[2];
	if (p[0] == 'r') {
		g_random_pkt_size = TRUE;
		pkt_size = MAX_PKTSIZE;
		printf("using random packet sizes...\n");
		if (strlen(argv[2]) > 1) {
			int rnd_seed = atoi(&p[1]);
			printf("initializing with random seed=%d\n", rnd_seed);
			srand(rnd_seed);
                }
	} else {
	pkt_size = atoi(argv[2]);
		if (pkt_size < 8) {
			printf("minimum packet size is 8 bytes\n");
			return -1;
		}
		if (pkt_size % 512 == 0) {
			printf("packet size is multiple of 512, this would require ZLPs\n");
			return -1;
		}
	}
    } 
    printf("pkt_cnt = %d, pkt_size = %d bytes\n", pkt_cnt, pkt_size);
    usb_init();

    usb_find_busses();
    usb_find_devices();

    for (bus = usb_busses; bus; bus = bus->next)
	    for (dev = bus->devices; dev; dev = dev->next) {
		printf("descriptor.idVendor = 0x%x, descriptor.idProduct = 0x%x\n", 
		dev->descriptor.idVendor, dev->descriptor.idProduct);
		if (dev->descriptor.idVendor == DONGLE_VENDOR &&
		    dev->descriptor.idProduct == DONGLE_PRODUCT_CDC) {
		    ep_data_in  = CDC_EP_DATA_IN;
		    ep_data_out = CDC_EP_DATA_OUT;
		    goto done;
		}
		if (dev->descriptor.idVendor == DONGLE_VENDOR &&
		    dev->descriptor.idProduct == DONGLE_PRODUCT_BDC) {
		    ep_data_in  = BDC_EP_DATA_IN;
		    ep_data_out = BDC_EP_DATA_OUT;
		    goto done;
		}
	    }	    
done:
    if (!dev) {
	printf("Device not found\n");
	goto done;
    }

    if ((udev = usb_open(dev)) == NULL) {
	fprintf(stderr, "Could not open device (permission?)\n");
	goto end;
    }

    buf = (char*)malloc(desc_size);
    if (!buf) {
	printf("Unable to allocate query buf, quitting!\n");
	goto end;
    }

    if (dev->descriptor.iManufacturer) {
	if (usb_get_string_simple(udev, dev->descriptor.iManufacturer, buf, sizeof(buf)) <= 0)
	    fprintf(stderr, "Unable to determine manufacturer\n");
	else
	    printf("Manufacturer: %s\n", buf);
    }

    if (dev->descriptor.iProduct) {
	if (usb_get_string_simple(udev, dev->descriptor.iProduct, buf, sizeof(buf)) <= 0)
	    fprintf(stderr, "Unable to determine product\n");
	else
	    printf("Product: %s\n", buf);
    }

    if (dev->descriptor.iSerialNumber) {
	if (usb_get_string_simple(udev, dev->descriptor.iSerialNumber, buf, sizeof(buf)) <= 0)
	    fprintf(stderr, "Unable to determine product\n");
	else
	    printf("Serial Number: %s\n", buf);
    }

    printf("Number of configurations: %d\n", dev->descriptor.bNumConfigurations);


    /*
     * Set up
     */

    usb_set_debug(10);


    printf("Claim interfaces\n");

    if (usb_claim_interface(udev, 0) < 0) {		/* CDC-control */
	fprintf(stderr, "Could not claim interface 0\n");
	goto end;
    }


    printf("Using endpoint 0x%02X for host->dongle traffic (BULK-OUT)\n", 	ep_data_out);
    printf("Using endpoint 0x%02X for host<-dongle traffic (BULK-IN)\n", 	ep_data_in);
    
    /*
     * Create a thread to read echo replies
     */

    if ((rv = pthread_create(&dt, NULL, reply_thread, (void *)udev)) < 0) {
	fprintf(stderr,
	        "Could not create reply thread: %s\n",
	        strerror(errno));
	goto end;
    }
    /*
     * Send some packets
     */
    stime = double_time();
    if (pkt_size < seq_size)
	pkt_size = seq_size;
    buf = (char*)malloc(pkt_size);
    if (!buf) {
	printf("Unable to allocate send buffer, quitting!\n");
	goto end;
    }

    for (i = 0; i < pkt_cnt; i++) {
	int n_pkt_bytes;

	bcopy(&i, buf, 4);				/* buf[0:3] is a sequence number */
	for (j = 4; j < pkt_size; j++)
		buf[j] = j;

	n_pkt_bytes = pkt_size;
	if (g_random_pkt_size == TRUE) {
		n_pkt_bytes = rnd(8, pkt_size);		/* need minimum 8 bytes for header */
		if (n_pkt_bytes % 512 == 0) {
			n_pkt_bytes++;			/* avoid need to send ZLPs */
		}
	}
	bcopy(&n_pkt_bytes, buf+4, 4);			/* buf[4:7] is packet length */

//	printf("tx pkt bytes = %d on endpoint %d\n", n_pkt_bytes, ep_data_out);

	if (!(i % 25))
		printf("Sent %d packets\n", i);

	if ((rv = usb_bulk_write(udev, ep_data_out,
				buf, n_pkt_bytes,
				3000 /*timeout in msec*/)) < 0) {
	    fprintf(stderr,
	            "usb_bulk_write failed: errno=%d (%s)\n",
	            errno, strerror(errno));
	    printf("Sent %d packets before this error occurred.\n", i);
	    goto end;
	}
//		usleep(100000);
    }
    printf("Sent %d packets\n", pkt_cnt);
    usleep(10000); /* give receive thread time to process packets */

end:
    rt_exit = 1;
    etime = double_time();
    //printf("Waiting for reply thread to exit\n");

    while (!rt_exited)
	usleep(10000);

    printf("\n%d writes; %f writes/sec\n",
           pkt_cnt, pkt_cnt / (etime - stime));
    printf("%d bytes total; %f bytes/sec\n",
           (pkt_cnt * pkt_size), (pkt_cnt * pkt_size) / (etime - stime));
    printf("%d packets received\n", rt_count);

    if (buf)
        free(buf);

    /* Close device */
    usb_close(udev);

    exit(0);

    /*NOTREACHED*/
    return 0;
}
