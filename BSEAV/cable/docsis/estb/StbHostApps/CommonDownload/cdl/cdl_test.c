#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "cdl_common.h"
#include "cdl_service.h"

#ifdef DSG_SUPPORT
extern int InitializeDsgCcApplication(void);
#endif
void usage( char *str) {
	printf("test tool for common download\n");
	printf("Usage: %s options\n",str);
	printf("options:\n");
	printf("   -d <monolithic image>: display the program header\n");
	printf("   -v <monolithic image>: verify the program header, and the content\n");
	printf("   -g <monolithic image>: generate program header, param file is required\n");
        printf("   -l <secure header length>: specify the secure header length\n");
	printf("   -p <param.txt> contains the CFE image partition name\n and file name, e.g\n");
	printf("       # cfe partition, file name\n");
	printf("       cfe.ecmboot, ./signedsecure3255.bin\n");
	printf("       cfe.docsis0, ./ecram_sto.bin\n");
#ifndef HOST_ONLY
	printf("   -w : wait to download the image\n");
	printf("   -u <monolithic image>: upgrade flash with monolithic image\n");	
	printf("   -a <signed content>: authenticate the digital signature and content\n");
	printf("       <signed content> is the monolithic image with the digital signature stripped off\n");
	printf("       must be accompanied with -c and -s\n");
	printf("   -c <cvc file>: CVC file used for authentication\n");
	printf("   -s <secure header file >: secure header (digtial signature) of a signed image\n");
	printf("   As an option, you can set the boot up command in STARTUP environment variable\n");
#endif
}


int main(int argc, char *argv[]) {
#define ACT_DISPLAY_PROG_HEADER  0
#define ACT_GEN_IMAGE            1
#define ACT_VERIFY_IMAGE         2
#define ACT_UPGRADE_IMAGE        3
#define ACT_AUTHENTICATE_IMAGE   4
#define ACT_WAIT_IMAGE   5

	int action = ACT_DISPLAY_PROG_HEADER;
	char monolithic_image_fn[128]="";
	char fn_cvc[MAX_DEV_STR_LEN]="";
	char fn_sec_header[MAX_DEV_STR_LEN]="";
	char param_fn[128]="";
	int secure_header_len = 0;
#ifdef DSG_SUPPORT
	pthread_t dsgcc_thread;
	int dsgcc_thread_started = 0;
#endif
	cdl_service_params cdl_params;


	int i = 0;
	if (argc == 1) {
		goto err;
	}
	i = 1;

	while (i < argc) {
		if (strncmp(argv[i], "-d", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(monolithic_image_fn, argv[i], 128);
			action = ACT_DISPLAY_PROG_HEADER;
		} else if (strncmp(argv[i], "-g", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(monolithic_image_fn, argv[i], 128);
			action = ACT_GEN_IMAGE;
		} else if (strncmp(argv[i], "-p", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(param_fn, argv[i], 128);
		} else if (strncmp(argv[i], "-v", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(monolithic_image_fn, argv[i], 128);
			action = ACT_VERIFY_IMAGE;
		} else if (strncmp(argv[i], "-a", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(monolithic_image_fn, argv[i], 128);
			action = ACT_AUTHENTICATE_IMAGE;
		} else if (strncmp(argv[i], "-u", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(monolithic_image_fn, argv[i], 128);
			action = ACT_UPGRADE_IMAGE;
		} else if (strncmp(argv[i], "-l", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			secure_header_len = strtol(argv[i], NULL, 0);

		} else if (strncmp(argv[i], "-c", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(fn_cvc, argv[i], MAX_DEV_STR_LEN);
		} else if (strncmp(argv[i], "-s", 2) == 0) {
			i++;
			if (i>= argc) goto err;
			strncpy(fn_sec_header, argv[i], MAX_DEV_STR_LEN);
		} else if (strncmp(argv[i], "-w", 2) == 0) {
			action = ACT_WAIT_IMAGE;
		} else {
			printf("invalid option %s\n", argv[1]);
			goto err;
		}
		i++;
	}

#ifdef DSG_SUPPORT
	if (action == ACT_AUTHENTICATE_IMAGE || action == ACT_WAIT_IMAGE) {
		pthread_create(&dsgcc_thread, NULL, (void *(*)(void *))(InitializeDsgCcApplication), NULL); 
		dsgcc_thread_started = 1;
	}
#endif


	cdl_service_init();
	if (action != ACT_WAIT_IMAGE) {
		cdl_service_get_params(&cdl_params);
		snprintf(&cdl_params.storage_file_downloaded[0], MAX_DEV_STR_LEN, "%s", monolithic_image_fn);
		if (action == ACT_AUTHENTICATE_IMAGE) {
			if (strncmp(fn_cvc, "", MAX_DEV_STR_LEN)) {
				snprintf(&cdl_params.debug.fn_cvc[0], MAX_DEV_STR_LEN, "%s", fn_cvc);
			}
			if (strncmp(fn_sec_header, "", MAX_DEV_STR_LEN)) {
				snprintf(&cdl_params.debug.fn_sec_header[0], MAX_DEV_STR_LEN, "%s", fn_sec_header);
			}
			cdl_params.debug.authenticate_only = 1;
		}
		cdl_service_set_params(&cdl_params);
	}
	cdl_service_start();
	printf("action ==== %d\n", action);
	if (action == ACT_DISPLAY_PROG_HEADER) {
		/* display image header */
		cdl_service_print_image_header(secure_header_len);
	} else if (action == ACT_GEN_IMAGE) {
		/* generate image */
		if (cdl_service_generate_image(param_fn) < 0) goto err;
	} else if (action == ACT_VERIFY_IMAGE) {
		/* verify image */
		if (cdl_service_verify_image(param_fn) < 0) goto err;
#ifndef HOST_ONLY
	} else if (action == ACT_UPGRADE_IMAGE) {
		/* upgrade image */
		if (cdl_service_upgrade_image(secure_header_len) < 0) goto err;
	} else if (action == ACT_AUTHENTICATE_IMAGE) {
		/* authenticate image */
#ifdef DSG_SUPPORT
		printf("sleep 5 seocnds, wait for DSGCC to come up\n");
		sleep(5);
		if (cdl_service_authenticate_image(1) < 0) goto err;
#else
		printf("Need dsgcc support!\n"); goto err;
#endif
	} else if (action == ACT_WAIT_IMAGE) {
#ifdef DSG_SUPPORT
		printf("Wait for trigger from eCM or eSTB\n");
#else
		printf("Need dsgcc support!\n"); goto err;
#endif
	} else {
		printf("invalid action %d\n", action);
		goto err;
#endif /* HOST_ONLY */
	}
	
#ifdef DSG_SUPPORT
	while (dsgcc_thread_started) {
		sleep(5);
	}
#endif
	return 0;
 err:
	usage(argv[0]);
	return -1;
}
