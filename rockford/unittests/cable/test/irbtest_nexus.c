/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/


 /**
 *Summary. this is the application used to demonstrate XMP2 IR
 *Protocol. Please refer to UEI spec
 **/

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "string.h"
#include "input_xmp.h"
#define MAX_INPUT_SIZE 300

xmp_output_t led;
void callback(void *context)
{
	BKNI_SetEvent((BKNI_EventHandle)context);
}

/* return 0 if succeed, skip all comment lines,  -1 if error */
static int scanline(FILE *fp, char * line)
{
    char * ptr1, * ptr2;

    while (fgets(line, 1024, fp)) {
	if ( (ptr1 = strchr(line, '#')) != NULL) {
	    continue;
	} else {
	    int i = 0;
	    int len = strlen(line);
	    while (i < len) {
		if ( !(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r'))
		    return 0;
		i++;
	    }
	    continue;
	}
    }
    return -1;
}

struct context {
xmp_input_t remote;
xmp_input_settings settings;
BKNI_EventHandle event;
BKNI_EventHandle xmp_event;
} g_context;


void xmp_callback(void *context)
{
	BKNI_SetEvent((BKNI_EventHandle)context);
}

void *remote_XMP1(void *context);
void main( void )
{
	xmp_result rc;
	BERR_Code rc_pi;
	xmp_result bxmp_rc;
	xmp_result 		  receive_result;
	xmp_input_settings xmp_settings;
	BKNI_EventHandle  hEvent;
	size_t		  	  input_size = MAX_INPUT_SIZE;
	uint8_t           input_data[MAX_INPUT_SIZE];
	uint8_t 		  output_data[MAX_INPUT_SIZE];
	uint32_t 		  datapacket = 0;
	uint32_t 		  output_size = 0;
	uint32_t 		  checker = 0;
	uint16_t i=0;
	uint8_t test_case;
	pthread_t thread;
	FILE *fp;
	char line[1024];
	char option[40];

/* data filled in an array. 0x0 is needed to activate the transceiver which is not needed in our case
   Also if the array contains odd number of bytes, add 0x00 as the last byte to make it even,
   since 2 bytes are sent at a time.
 */
	const uint8_t case1_reference_data_1[]={0x43, 0x01, 0x25, 0x42, 0x03, 0x80, 0xe5, 0x1a, 0x00, 0x1d, 0x00, 0x10, 0x00,
			 0x00,0xca, 0x00, 0xff, 0x80, 0x25, 0x40, 0x90, 0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10,
			 0x48,0xc8, 0x28, 0x08, 0x88, 0xa8, 0x34, 0xd0, 0xdc, 0xbc};
	const uint8_t case1_reference_data_2[]={0x43, 0x01, 0x02, 0x43, 0x03, 0x00};
	const uint8_t case1_reference_data_3[]={0x43, 0x01, 0x05, 0x47, 0x03, 0x80, 0x80, 0x03};

	const uint8_t case2_reference_data_1[]={0x43, 0x01, 0x84, 0x42, 0x03, 0x80, 0x14, 0xeb, 0x00, 0xb4, 0x16, 0x10, 0x54,
			0x00, 0x51, 0x00, 0xd1, 0x91, 0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x11, 0x91,
			0x81, 0x41, 0x43, 0xa0, 0x11, 0x8b, 0x09, 0x00, 0xa6, 0x43, 0xb4, 0x43, 0x64, 0x32, 0x71, 0x32, 0x76,
            0x03, 0x01, 0xeb, 0x0b, 0x56, 0x04, 0xfe, 0x76, 0x00, 0x01, 0x6B, 0x03, 0x46, 0x04, 0x01, 0xE6, 0xF8,
			0x00, 0x77, 0x51, 0x4C, 0x0D, 0x88, 0x04, 0x98, 0x03, 0xA8, 0xC8, 0x60, 0xCA, 0x1C, 0x16, 0x37, 0x8F,
			0x02, 0x1C, 0x12, 0x08, 0xCA, 0xB2, 0x08, 0xDB, 0x20, 0x37, 0x50, 0x02, 0x1E, 0x1E, 0x77, 0x50, 0x68,
			0xC8, 0x10, 0xC6, 0xC0, 0xC2, 0xB2, 0x26, 0x37, 0x2F, 0x0C, 0xE5, 0xC1, 0x06, 0x1E, 0xE5, 0xC1, 0x07,
			0x06, 0x06, 0x08, 0x1C, 0x06, 0x8B, 0x13, 0x77, 0x51, 0x37, 0x70, 0x0C, 0xE5, 0xC1, 0x06, 0x1E, 0xE5,
			0xC1, 0x07, 0x14, 0x00
			};
	const uint8_t case2_reference_data_2[]={0x43, 0x01, 0x3C, 0x42, 0x04, 0x00, 0x26, 0x06, 0x18, 0x1C, 0x06, 0x77, 0x70,
			0xF6, 0x01, 0x67, 0xE5, 0xc1, 0xF9, 0x1E, 0xE6, 0xFA, 0x8A, 0x37, 0x51, 0x0B, 0xE5, 0xC1, 0xF4, 0x46,
			0xF3, 0x04, 0x46, 0xEF, 0x20, 0x77, 0x71, 0x77, 0x7F, 0x10, 0xC9, 0x10, 0xC8, 0xC0, 0xCA, 0x4A, 0x9E,
			0x77, 0x70, 0xC6, 0xF8, 0xE2, 0x90, 0xF6, 0x01, 0x58, 0xF6, 0x01, 0x0A, 0x7B, 0x81, 0xAF, 0xC0, 0x00
		    };
	const uint8_t case2_reference_data_3[]={0x43, 0x01, 0x02, 0x43, 0x03, 0x00};

		//RECORDGET FOR TYPE = REC_MODE_SETUP (00)
	const uint8_t case4_reference_data_1[]={0x00, 0x07, 0x81, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0x78, 0x00};//Valid; should return the records setup of the remote

	//Set T0000 to TV & unlock channel (valid)
	const uint8_t case5_reference_data_1[]={0x00, 0x12, 0x82, 0x00, 0x10, 0x00, 0xFF, 0x02, 0x00, 0x01, 0x00, 0x00, 0xFF,
			0x02, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0x7C};//Valid test should return zero
	//Set T1081 to TV & unlock channel (invalid)
	const uint8_t case5_reference_data_2[]={0x00, 0x12, 0x82, 0x00, 0x10, 0x00, 0xFF, 0x02, 0x00, 0x01, 0x04, 0x39, 0xFF,
			0x02, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0x41};//Invalid test should return error
	//Set T0000 to AUX3 ,lock volume to STB lock channel to TV (valid)
	const uint8_t case5_reference_data_3[]={0x00, 0x12, 0x82, 0x00, 0x10, 0x00, 0xFF, 0x08, 0x00, 0x01, 0x00, 0x00, 0xFF,
			0x01, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0x75};//Valid test should return zero

	//Set Upgrade R0014 via RECORDSET cmd
	const uint8_t case6_reference_data_1[]= {0x00, 0x3D, 0x82, 0x00, 0x3B, 0x10, 0xFF, 0x00, 0x00, 0x00, 0x16, 0x03, 0x00,
			0x0E, 0x01, 0x01, 0x00, 0xE2, 0x00, 0xA1, 0xDA, 0x4F, 0x00, 0x00, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xF7, 0x77, 0x40, 0x84, 0x11, 0x8B, 0x14, 0xA5, 0x55, 0x08, 0x08, 0x01, 0x12, 0x00, 0xF0, 0x01, 0x12, 0x03,
			0x02, 0x32, 0xB4, 0x10, 0xB0, 0x08, 0x39, 0x00, 0x02, 0x8D, 0x01, 0x46, 0x47, 0x7A, 0x00};//Valid test should return zero status.
	rc = xmp_init();
	g_context.remote = xmp_input_open();
	printf("Opened remote\n");
    led = xmp_output_open();
	printf("Opened LED\n");
	BKNI_CreateEvent(&g_context.event);
	BKNI_CreateEvent(&g_context.xmp_event);
    fp = fopen("xmp.txt", "rt");
	if (fp == NULL){
		printf("####No input file opened\n");
		return;
	}
	if (g_context.remote) {
	xmp_input_get_settings(g_context.remote, &xmp_settings);
	while(!feof(fp)) {
				checker = ftell(fp);
					if (scanline(fp, line) == 0){
						sscanf(line, "%s", option);
						if(!(strcmp(option, "xmp1_owner"))){
							sscanf(line,"xmp1_owner 0x%x\n",     &xmp_settings.xmp1_owner);
						}
						else{
							fseek(fp, checker, SEEK_SET);
						}
					}

					checker = ftell(fp);
					if (scanline(fp, line) == 0){
						sscanf(line, "%s", option);
						if(!(strcmp(option, "xmp1_registry"))){
							sscanf(line,"xmp1_registry 0x%x\n",     &xmp_settings.xmp1_registry);
						}
						else{
							fseek(fp, checker, SEEK_SET);
						}

					}

					checker = ftell(fp);
					if (scanline(fp, line) == 0){
						sscanf(line, "%s", option);
						if(!(strcmp(option, "xmp2_owner"))){
							sscanf(line,"xmp2_owner 0x%x\n",     &xmp_settings.xmp2_owner);
						}
						else{
							fseek(fp, checker, SEEK_SET);
						}
					}
					checker = ftell(fp);
					if (scanline(fp, line) == 0){
						sscanf(line, "%s", option);
						if(!(strcmp(option, "xmp2_tag"))){
							sscanf(line,"xmp2_tag 0x%x\n",     &xmp_settings.xmp2_tag);
						}
						else{
							fseek(fp, checker, SEEK_SET);
						}

					}

					checker = ftell(fp);
					if (scanline(fp, line) == 0){
						sscanf(line, "%s", option);
						if(!(strcmp(option, "xmp2_remote_registry"))){
							sscanf(line,"xmp2_remote_registry 0x%x\n",     &xmp_settings.xmp2_remote_registry);
						}
						else{
							fseek(fp, checker, SEEK_SET);
						}

					}
					checker = ftell(fp);
					if (scanline(fp, line) == 0){
						sscanf(line, "%s", option);
						if(!(strcmp(option, "xmp2_transceiver_registry"))){
							sscanf(line,"xmp2_transceiver_registry 0x%x\n",     &xmp_settings.xmp2_transceiver_registry);
						}
						else{
							fseek(fp, checker, SEEK_SET);
						}

					}

	}
	xmp_settings.xmp2remote = true;
	xmp_settings.data_ready_callback = xmp_callback;
	xmp_settings.callback_context = g_context.xmp_event;
	xmp_input_set_settings(g_context.remote, &xmp_settings);
	}

	fclose(fp);

	if (pthread_create(&thread, NULL, remote_XMP1, &g_context))
		printf("Unable to acquire thread\n");

	printf("Main Menu\n");
	printf("The available test cases are\n");
	printf("Cases 0,1,2 are for the demo remote. Cases 3,4,5,6 are for the titan remote\n");
	printf("0.DEMO: software version request\n");
	printf("1.DEMO: Data Download, Data Download End and Upload\n");
	printf("2.DEMO: Data Download and Data Download End\n");
	printf("3.TITAN: software version request\n");
	printf("4.TITAN: UNKNOWN\n");
	printf("5.TITAN: UNKNOWN\n");
	printf("6.TITAN: UNKNOWN\n");

		printf("Enter the Test case\n");
		scanf("%d", &test_case);
		getchar();
		switch(test_case) {
		case 0:
			for(i=0;i<MAX_INPUT_SIZE;i++) {
				input_data[i] = 0;
				output_data[i] = 0;
			}
			getchar();
			input_size = 6;
			input_data[0] = 0x43;
			input_data[1] = 0x01;
			input_data[2] = 0x02;
			input_data[3] = 0x48;
			input_data[4] = 0x08;
			input_data[5] = 0x00;
			bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);

			if(bxmp_rc == xmp_ok) {
				printf("XMP SEND: Data Send Successful\n");
			}
			else if(bxmp_rc == xmp_external_error) {
				printf("XMP SEND: Fatal Error\n");
			}
			else if(bxmp_rc == xmp_timeout) {
				printf("XMP SEND: Error due to Timeout\n");
			}
			else if(bxmp_rc == xmp_invalid_parameter) {
				printf("XMP SEND: Checksum Error\n");
			}
			else{
				printf("XMP SEND: Unknown Cause failure\n");
			}


			receive_result = xmp_ok; //initialize to a known condition
			do{
				rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
				receive_result = xmp_input_receive(g_context.remote, &output_data[0],
													 MAX_INPUT_SIZE, &output_size, &datapacket);
				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
				if(rc_pi) {
					receive_result = xmp_timeout;
					break;
				}
			}while ((datapacket) && ((datapacket & 0x00100000)));

			if(receive_result == xmp_timeout) {
				printf("XMP RECEIVE: Timeout Occured\n");
			}
			else if(receive_result == xmp_ok) {
				printf("XMP RECEIVE: Data Received Successful\n");
			}
			else if(receive_result == xmp_invalid_parameter) {
				//checksum error can also be due to incorrect number of bytes
				printf("XMP RECEIVE: Checksum error in output\n");
			}
			else{
				printf("XMP RECEIVE: Unknown Cause failure\n");
			}


			for(i=0;i<output_size;i++) {
				printf("data received is %x\n", output_data[i]);
			}
			printf("Size is %d\n", output_size);

			break;
	case 1:
			// flush the input buffer and output buffer
			for(i=0;i<MAX_INPUT_SIZE;i++) {
				input_data[i] = 0;
				output_data[i] = 0;
			}

			getchar();
			input_size = 40;
			for(i=0;i<input_size;i++) {
				input_data[i] = case1_reference_data_1[i];
			}

			bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);

			if(bxmp_rc == xmp_ok) {
				printf("XMP SEND: Data Send Successful\n");
			}
			else if(bxmp_rc == xmp_external_error) {
				printf("XMP SEND: Fatal Error\n");
			}
			else if(bxmp_rc == xmp_timeout) {
				printf("XMP SEND: Error due to Timeout\n");
			}
			else if(bxmp_rc == xmp_invalid_parameter) {
				printf("XMP SEND: Checksum Error\n");
			}
			else{
				printf("XMP SEND: Unknown Cause failure\n");
			}

			receive_result = xmp_ok; //initialize to a known condition
			do{
				rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
				receive_result = xmp_input_receive(g_context.remote, &output_data[0],
													 MAX_INPUT_SIZE, &output_size, &datapacket);
				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
				if(rc_pi) {
					receive_result = xmp_timeout;
					break;
				}
			}while ((datapacket) && ((datapacket & 0x00100000)));

			if(receive_result == xmp_timeout) {
				printf("XMP RECEIVE: Timeout Occured\n");
			}
			else if(receive_result == xmp_ok) {
				printf("XMP RECEIVE: Data Received Successful\n");
			}
			else if(receive_result == xmp_invalid_parameter) {
				//checksum error can also be due to incorrect number of bytes
				printf("XMP RECEIVE: Checksum error in output\n");
			}
			else{
				printf("XMP RECEIVE: Unknown Cause failure\n");
			}


			for(i=0;i<output_size;i++) {
				printf("data received is %x\n", output_data[i]);
			}
			printf("Size is %d\n", output_size);


			// flush the input buffer and output buffer
			for(i=0;i<MAX_INPUT_SIZE;i++) {
				input_data[i] = 0;
				output_data[i] = 0;
			}
			usleep(10000);
			input_size = 6;
			for(i=0;i<input_size;i++) {
				input_data[i] = case1_reference_data_2[i];
			}

			bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
			if(bxmp_rc == xmp_ok) {
				printf("XMP SEND: Data Send Successful\n");
			}
			else if(bxmp_rc == xmp_external_error) {
				printf("XMP SEND: Fatal Error\n");
			}
			else if(bxmp_rc == xmp_timeout) {
				printf("XMP SEND: Error due to Timeout\n");
			}
			else if(bxmp_rc == xmp_invalid_parameter) {
				printf("XMP SEND: Checksum Error\n");
			}
			else{
				printf("XMP SEND: Unknown Cause failure\n");
			}


			receive_result = xmp_ok; //initialize to a known condition
			do{
				rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
				receive_result = xmp_input_receive(g_context.remote, &output_data[0],
													 MAX_INPUT_SIZE, &output_size, &datapacket);
				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
				if(rc_pi) {
					receive_result = xmp_timeout;
					break;
				}
			}while ((datapacket) && ((datapacket & 0x00100000)));

			if(receive_result == xmp_timeout) {
				printf("XMP RECEIVE: Timeout Occured\n");
			}
			else if(receive_result == xmp_ok) {
				printf("XMP RECEIVE: Data Received Successful\n");
			}
			else if(receive_result == xmp_invalid_parameter) {
				//checksum error can also be due to incorrect number of bytes
				printf("XMP RECEIVE: Checksum error in output\n");
			}
			else{
				printf("XMP RECEIVE: Unknown Cause failure\n");
			}


			for(i=0;i<output_size;i++) {
				printf("data received is %x\n", output_data[i]);
			}
			printf("Size is %d\n", output_size);


			// flush the input buffer and output buffer
			for(i=0;i<MAX_INPUT_SIZE;i++) {
				input_data[i] = 0;
				output_data[i] = 0;
			}
			usleep(10000);
			input_size = 8;
			for(i=0;i<input_size;i++) {
				input_data[i] = case1_reference_data_3[i];
			}

			bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
			if(bxmp_rc == xmp_ok) {
				printf("XMP SEND: Data Send Successful\n");
			}
			else if(bxmp_rc == xmp_external_error) {
				printf("XMP SEND: Fatal Error\n");
			}
			else if(bxmp_rc == xmp_timeout) {
				printf("XMP SEND: Error due to Timeout\n");
			}
			else if(bxmp_rc == xmp_invalid_parameter) {
				printf("XMP SEND: Checksum Error\n");
			}
			else{
				printf("XMP SEND: Unknown Cause failure\n");
			}


			receive_result = xmp_ok; //initialize to a known condition
			do{
				rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
				receive_result = xmp_input_receive(g_context.remote, &output_data[0],
													 MAX_INPUT_SIZE, &output_size, &datapacket);
				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
				if(rc_pi) {
					receive_result = xmp_timeout;
					break;
				}
			}while ((datapacket) && ((datapacket & 0x00100000)));

			if(receive_result == xmp_timeout) {
				printf("XMP RECEIVE: Timeout Occured\n");
			}
			else if(receive_result == xmp_ok) {
				printf("XMP RECEIVE: Data Received Successful\n");
			}
			else if(receive_result == xmp_invalid_parameter) {
				//checksum error can also be due to incorrect number of bytes
				printf("XMP RECEIVE: Checksum error in output\n");
			}
			else{
				printf("XMP RECEIVE: Unknown Cause failure\n");
			}

#if 0
			for(i=0;i<output_size;i++) {
				printf("data received is %x\n", output_data[i]);
			}
#endif
			printf("Size is %d\n", output_size);

			break;

	case 2:
		// flush the input buffer and output buffer
		for(i=0;i<MAX_INPUT_SIZE;i++) {
			input_data[i] = 0;
			output_data[i] = 0;
		}

		getchar();
		input_size = 136;
		for(i=0;i<input_size;i++) {
			input_data[i] = case2_reference_data_1[i];
		}

		bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
		if(bxmp_rc == xmp_ok) {
			printf("XMP SEND: Data Send Successful\n");
		}
		else if(bxmp_rc == xmp_external_error) {
			printf("XMP SEND: Fatal Error\n");
		}
		else if(bxmp_rc == xmp_timeout) {
			printf("XMP SEND: Error due to Timeout\n");
		}
		else if(bxmp_rc == xmp_invalid_parameter) {
			printf("XMP SEND: Checksum Error\n");
		}
		else{
			printf("XMP SEND: Unknown Cause failure\n");
		}



		receive_result = xmp_ok; //initialize to a known condition
		do{
			rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
			receive_result = xmp_input_receive(g_context.remote, &output_data[0],
												 MAX_INPUT_SIZE, &output_size, &datapacket);
			if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
				datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
			}

			if(rc_pi) {
				receive_result = xmp_timeout;
				break;
			}
		}while ((datapacket) && ((datapacket & 0x00100000)));

		if(receive_result == xmp_timeout) {
			printf("XMP RECEIVE: Timeout Occured\n");
		}
		else if(receive_result == xmp_ok) {
			printf("XMP RECEIVE: Data Received Successful\n");
		}
		else if(receive_result == xmp_invalid_parameter) {
			//checksum error can also be due to incorrect number of bytes
			printf("XMP RECEIVE: Checksum error in output\n");
		}
		else{
			printf("XMP RECEIVE: Unknown Cause failure\n");
		}


		for(i=0;i<output_size;i++) {
			printf("data received is %x\n", output_data[i]);
		}
		printf("Size is %d\n", output_size);

		// flush the input buffer and output buffer
		for(i=0;i<MAX_INPUT_SIZE;i++) {
			input_data[i] = 0;
			output_data[i] = 0;
		  }
		usleep(10000);
		input_size = 64;
		for(i=0;i<input_size;i++) {
			input_data[i] = case2_reference_data_2[i];
		}

		bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);

		if(bxmp_rc == xmp_ok) {
			printf("XMP SEND: Data Send Successful\n");
		}
		else if(bxmp_rc == xmp_external_error) {
			printf("XMP SEND: Fatal Error\n");
		}
		else if(bxmp_rc == xmp_timeout) {
			printf("XMP SEND: Error due to Timeout\n");
		}
		else if(bxmp_rc == xmp_invalid_parameter) {
			printf("XMP SEND: Checksum Error\n");
		}
		else{
			printf("XMP SEND: Unknown Cause failure\n");
		}

		receive_result = xmp_ok; //initialize to a known condition
		do{
			rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
			receive_result = xmp_input_receive(g_context.remote, &output_data[0],
												 MAX_INPUT_SIZE, &output_size, &datapacket);
			if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
				datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
			}

			if(rc_pi) {
				receive_result = xmp_timeout;
				break;
			}
		}while ((datapacket) && ((datapacket & 0x00100000)));

		if(receive_result == xmp_timeout) {
			printf("XMP RECEIVE: Timeout Occured\n");
		}
		else if(receive_result == xmp_ok) {
			printf("XMP RECEIVE: Data Received Successful\n");
		}
		else if(receive_result == xmp_invalid_parameter) {
			//checksum error can also be due to incorrect number of bytes
			printf("XMP RECEIVE: Checksum error in output\n");
		}
		else{
			printf("XMP RECEIVE: Unknown Cause failure\n");
		}

		for(i=0;i<output_size;i++) {
			printf("data received is %x\n", output_data[i]);
		}
		printf("Size is %d\n", output_size);


		// flush the input buffer and output buffer
		for(i=0;i<MAX_INPUT_SIZE;i++) {
			input_data[i] = 0;
			output_data[i] = 0;
		}
		usleep(10000);
		input_size = 6;
		for(i=0;i<input_size;i++) {
			input_data[i] = case2_reference_data_3[i];
		}

		bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
		if(bxmp_rc == xmp_ok) {
			printf("XMP SEND: Data Send Successful\n");
		}
		else if(bxmp_rc == xmp_external_error) {
			printf("XMP SEND: Fatal Error\n");
		}
		else if(bxmp_rc == xmp_timeout) {
			printf("XMP SEND: Error due to Timeout\n");
		}
		else if(bxmp_rc == xmp_invalid_parameter) {
			printf("XMP SEND: Checksum Error\n");
		}
		else{
			printf("XMP SEND: Unknown Cause failure\n");
		}


		receive_result = xmp_ok; //initialize to a known condition
		do{
			rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
			receive_result = xmp_input_receive(g_context.remote, &output_data[0],
												 MAX_INPUT_SIZE, &output_size, &datapacket);

			if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
				datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
			}
			if(rc_pi) {
				receive_result = xmp_timeout;
				break;
			}
		}while ((datapacket) && ((datapacket & 0x00100000)));

		if(receive_result == xmp_timeout) {
			printf("XMP RECEIVE: Timeout Occured\n");
		}
		else if(receive_result == xmp_ok) {
			printf("XMP RECEIVE: Data Received Successful\n");
		}
		else if(receive_result == xmp_invalid_parameter) {
			//checksum error can also be due to incorrect number of bytes
			printf("XMP RECEIVE: Checksum error in output\n");
		}
		else{
			printf("XMP RECEIVE: Unknown Cause failure\n");
		}


		for(i=0;i<output_size;i++) {
			printf("data received is %x\n", output_data[i]);
		}
		printf("Size is %d\n", output_size);

		break;

		/*#### Case 0: Get the IC type and application data address*************************************************************************************************************/
case 3:
			for(i=0;i<MAX_INPUT_SIZE;i++) {
				input_data[i] = 0;
				output_data[i] = 0;
			}
			getchar();
			input_size = 4;
			input_data[0] = 0x00;
			input_data[1] = 0x02;
			input_data[2] = 0x50;
			input_data[3] = 0x52;

			bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
			if(bxmp_rc == xmp_ok) {
				printf("XMP SEND: Data Send Successful\n");
			}
			else if(bxmp_rc == xmp_external_error) {
				printf("XMP SEND: Fatal Error\n");
			}
			else if(bxmp_rc == xmp_timeout) {
				printf("XMP SEND: Error due to Timeout\n");
			}
			else if(bxmp_rc == xmp_invalid_parameter) {
				printf("XMP SEND: Checksum Error\n");
			}
			else{
				printf("XMP SEND: Unknown Cause failure\n");
			}


			receive_result = xmp_ok; //initialize to a known condition
			do{
				rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
				receive_result = xmp_input_receive(g_context.remote, &output_data[0],
													 MAX_INPUT_SIZE, &output_size, &datapacket);
				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
				if(rc_pi) {
					receive_result = xmp_timeout;
					break;
				}
			}while ((datapacket) && ((datapacket & 0x00100000)));

			if(receive_result == xmp_timeout) {
				printf("XMP RECEIVE: Timeout Occured\n");
			}
			else if(receive_result == xmp_ok) {
				printf("XMP RECEIVE: Data Received Successful\n");
			}
			else if(receive_result == xmp_invalid_parameter) {
				//checksum error can also be due to incorrect number of bytes
				printf("XMP RECEIVE: Checksum error in output\n");
			}
			else{
				printf("XMP RECEIVE: Unknown Cause failure\n");
			}


			for(i=0;i<output_size;i++) {
				printf("data received is %x\n", output_data[i]);
			}
			printf("Size is %d\n", output_size);
			break;
		case 4:
			// flush the input buffer and output buffer
				for(i=0;i<MAX_INPUT_SIZE;i++) {
					input_data[i] = 0;
					output_data[i] = 0;
				}
				getchar();
				input_size = 10;
				for(i=0;i<input_size;i++) {
					input_data[i] = case4_reference_data_1[i];
				}

				bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
				if(bxmp_rc == xmp_ok) {
					printf("XMP SEND: Data Send Successful\n");
				}
				else if(bxmp_rc == xmp_external_error) {
					printf("XMP SEND: Fatal Error\n");
				}
				else if(bxmp_rc == xmp_timeout) {
					printf("XMP SEND: Error due to Timeout\n");
				}
				else if(bxmp_rc == xmp_invalid_parameter) {
					printf("XMP SEND: Checksum Error\n");
				}
				else{
					printf("XMP SEND: Unknown Cause failure\n");
				}


				receive_result = xmp_ok; //initialize to a known condition
				do{
					rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
					receive_result = xmp_input_receive(g_context.remote, &output_data[0],
														 MAX_INPUT_SIZE, &output_size, &datapacket);

				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
					if(rc_pi) {
						receive_result = xmp_timeout;
						break;
					}
				}while ((datapacket) && ((datapacket & 0x00100000)));

				if(receive_result == xmp_timeout) {
					printf("XMP RECEIVE: Timeout Occured\n");
				}
				else if(receive_result == xmp_ok) {
					printf("XMP RECEIVE: Data Received Successful\n");
				}
				else if(receive_result == xmp_invalid_parameter) {
					//checksum error can also be due to incorrect number of bytes
					printf("XMP RECEIVE: Checksum error in output\n");
				}
				else{
					printf("XMP RECEIVE: Unknown Cause failure\n");
				}


				for(i=0;i<output_size;i++) {
					printf("data received is %x\n", output_data[i]);
				}
				printf("Size is %d\n", output_size);

				break;
		case 5:
				// flush the input buffer and output buffer
				for(i=0;i<MAX_INPUT_SIZE;i++) {
					input_data[i] = 0;
					output_data[i] = 0;
				}
				getchar();
				input_size = 20;
				for(i=0;i<input_size;i++) {
					input_data[i] = case5_reference_data_1[i];
				}
				bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
				if(bxmp_rc == xmp_ok) {
					printf("XMP SEND: Data Send Successful\n");
				}
				else if(bxmp_rc == xmp_external_error) {
					printf("XMP SEND: Fatal Error\n");
				}
				else if(bxmp_rc == xmp_timeout) {
					printf("XMP SEND: Error due to Timeout\n");
				}
				else if(bxmp_rc == xmp_invalid_parameter) {
					printf("XMP SEND: Checksum Error\n");
				}
				else{
					printf("XMP SEND: Unknown Cause failure\n");
				}


				receive_result = xmp_ok; //initialize to a known condition
				do{
					rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
					receive_result = xmp_input_receive(g_context.remote, &output_data[0],
														 MAX_INPUT_SIZE, &output_size, &datapacket);
				if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
					datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
				}
					if(rc_pi) {
						receive_result = xmp_timeout;
						break;
					}
				}while ((datapacket) && ((datapacket & 0x00100000)));

				if(receive_result == xmp_timeout) {
					printf("XMP RECEIVE: Timeout Occured\n");
				}
				else if(receive_result == xmp_ok) {
					printf("XMP RECEIVE: Data Received Successful\n");
				}
				else if(receive_result == xmp_invalid_parameter) {
					//checksum error can also be due to incorrect number of bytes
					printf("XMP RECEIVE: Checksum error in output\n");
				}
				else{
					printf("XMP RECEIVE: Unknown Cause failure\n");
				}


				for(i=0;i<output_size;i++) {
					printf("data received is %x\n", output_data[i]);
				}
				printf("Size is %d\n", output_size);

				//*****************************next record set command
				usleep(10000);
				// flush the input buffer and output buffer
				for(i=0;i<MAX_INPUT_SIZE;i++) {
					input_data[i] = 0;
					output_data[i] = 0;
				}
				input_size = 20;
				for(i=0;i<input_size;i++) {
					input_data[i] = case5_reference_data_2[i];
				}
				bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
				if(bxmp_rc == xmp_ok) {
					printf("XMP SEND: Data Send Successful\n");
				}
				else if(bxmp_rc == xmp_external_error) {
					printf("XMP SEND: Fatal Error\n");
				}
				else if(bxmp_rc == xmp_timeout) {
					printf("XMP SEND: Error due to Timeout\n");
				}
				else if(bxmp_rc == xmp_invalid_parameter) {
					printf("XMP SEND: Checksum Error\n");
				}
				else{
					printf("XMP SEND: Unknown Cause failure\n");
				}


				receive_result = xmp_ok; //initialize to a known condition
				do{
					rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
					receive_result = xmp_input_receive(g_context.remote, &output_data[0],
														 MAX_INPUT_SIZE, &output_size, &datapacket);
					if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
						datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
					}
					if(rc_pi) {
						receive_result = xmp_timeout;
						break;
					}
				}while ((datapacket) && ((datapacket & 0x00100000)));

				if(receive_result == xmp_timeout) {
					printf("XMP RECEIVE: Timeout Occured\n");
				}
				else if(receive_result == xmp_ok) {
					printf("XMP RECEIVE: Data Received Successful\n");
				}
				else if(receive_result == xmp_invalid_parameter) {
					//checksum error can also be due to incorrect number of bytes
					printf("XMP RECEIVE: Checksum error in output\n");
				}
				else{
					printf("XMP RECEIVE: Unknown Cause failure\n");
				}


				for(i=0;i<output_size;i++) {
					printf("data received is %x\n", output_data[i]);
				}
				printf("Size is %d\n", output_size);
				//*****************************next record set command
				usleep(10000);
				// flush the input buffer and output buffer
				for(i=0;i<MAX_INPUT_SIZE;i++) {
					input_data[i] = 0;
					output_data[i] = 0;
				}
				input_size = 20;
				for(i=0;i<input_size;i++) {
					input_data[i] = case5_reference_data_3[i];
				}
				bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
				if(bxmp_rc == xmp_ok) {
					printf("XMP SEND: Data Send Successful\n");
				}
				else if(bxmp_rc == xmp_external_error) {
					printf("XMP SEND: Fatal Error\n");
				}
				else if(bxmp_rc == xmp_timeout) {
					printf("XMP SEND: Error due to Timeout\n");
				}
				else if(bxmp_rc == xmp_invalid_parameter) {
					printf("XMP SEND: Checksum Error\n");
				}
				else{
					printf("XMP SEND: Unknown Cause failure\n");
				}


				receive_result = xmp_ok; //initialize to a known condition
				do{
					rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
					receive_result = xmp_input_receive(g_context.remote, &output_data[0],
														 MAX_INPUT_SIZE, &output_size, &datapacket);
					if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
						datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
					}
					if(rc_pi) {
						receive_result = xmp_timeout;
						break;
					}
				}while ((datapacket) && ((datapacket & 0x00100000)));

				if(receive_result == xmp_timeout) {
					printf("XMP RECEIVE: Timeout Occured\n");
				}
				else if(receive_result == xmp_ok) {
					printf("XMP RECEIVE: Data Received Successful\n");
				}
				else if(receive_result == xmp_invalid_parameter) {
					//checksum error can also be due to incorrect number of bytes
					printf("XMP RECEIVE: Checksum error in output\n");
				}
				else{
					printf("XMP RECEIVE: Unknown Cause failure\n");
				}


				for(i=0;i<output_size;i++) {
					printf("data received is %x\n", output_data[i]);
				}
				printf("Size is %d\n", output_size);

				break;
		case 6:
			// flush the input buffer and output buffer
				for(i=0;i<MAX_INPUT_SIZE;i++) {
					input_data[i] = 0;
					output_data[i] = 0;
				}
				getchar();
				input_size = 64;
				for(i=0;i<input_size;i++) {
					input_data[i] = case6_reference_data_1[i];
				}

				bxmp_rc = xmp_input_send(g_context.remote, &input_data[0], input_size);
				if(bxmp_rc == xmp_ok) {
					printf("XMP SEND: Data Send Successful\n");
				}
				else if(bxmp_rc == xmp_external_error) {
					printf("XMP SEND: Fatal Error\n");
				}
				else if(bxmp_rc == xmp_timeout) {
					printf("XMP SEND: Error due to Timeout\n");
				}
				else if(bxmp_rc == xmp_invalid_parameter) {
					printf("XMP SEND: Checksum Error\n");
				}
				else{
					printf("XMP SEND: Unknown Cause failure\n");
				}

				receive_result = xmp_ok; //initialize to a known condition
				do{
					rc_pi = BKNI_WaitForEvent(g_context.xmp_event, 5000);
					receive_result = xmp_input_receive(g_context.remote, &output_data[0],
														 MAX_INPUT_SIZE, &output_size, &datapacket);
					if((datapacket & 0xffff) == xmp_settings.xmp2_remote_registry) {
						datapacket = 0x00100000;//manipulate the data. The loop shouldnt exit for registry number.
					}
					if(rc_pi) {
						receive_result = xmp_timeout;
						break;
					}
				}while ((datapacket) && ((datapacket & 0x00100000)));

				if(receive_result == xmp_timeout) {
					printf("XMP RECEIVE: Timeout Occured\n");
				}
				else if(receive_result == xmp_ok) {
					printf("XMP RECEIVE: Data Received Successful\n");
				}
				else if(receive_result == xmp_invalid_parameter) {
					//checksum error can also be due to incorrect number of bytes
					printf("XMP RECEIVE: Checksum error in output\n");
				}
				else{
					printf("XMP RECEIVE: Unknown Cause failure\n");
				}


		for(i=0;i<output_size;i++) {
			printf("data received is %x\n", output_data[i]);
		}
		printf("Size is %d\n", output_size);

		break;
	default:
		printf("wrong case\n");
	}

		BKNI_DestroyEvent(g_context.xmp_event);
		xmp_input_close(g_context.remote);
		xmp_uninit();
}

void *remote_XMP1(void *context_)
{
	struct context *context = (struct context *)context_;
	uint8_t i, n;
	if (context->remote) {

		xmp_input_get_settings(context->remote, &context->settings);
		context->settings.led_data_ready_callback = callback;
		context->settings.led_callback_context = context->event;
		xmp_input_set_settings(context->remote, &context->settings);

	}
	if (led) {
		xmp_output_display_message(led, "idle");
	}
	for(i=0;i<50;i++) {
		if (led) {
			xmp_output_set_led(led, n%4, n>=4?false:true);
			n = (n+1)%8;
		}

		/* wait for event or timeout to update leds */
		BKNI_WaitForEvent(context->event, BKNI_INFINITE);
	}

}
