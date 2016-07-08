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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_glb_control.h"
#include "bchp_common.h" 
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
 
unsigned int VIRTUAL_ADDRESS = 0xA0000000;

unsigned char send_otp_program_pattern(void)
{
	unsigned char status;
	
	/* check BSP_GLB_NONSECURE_GLB_IRDY */
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_IRDY) & 0x02 )  != 0x02);
			
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x300) = 0x10;
	/* seq */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x304) = 0x22;
	/* cont */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x308) = 0xabcdef00; 
	/* command ID to send OTP programming pattern. */ 	
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x30C) = 0xae55aa51;  
	/*  command length in bytes.*/
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x310) = 0x789a0024; 
	/* pattern */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x314) = 0xbc32f4ac;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x318) = 0xd18616b6;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x31C) = 0x9feb4d54;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x320) = 0x4a27bf4a;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x324) = 0xcf1c3178;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x328) = 0xe2db98a0;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x32C) = 0x24f64bba;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x330) = 0x7698e712;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x334) = 0x0000f48d;

	/* set BSP_GLB_NONSECURE_GLB_ILOAD2  */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_ILOAD2) = 0x01;

	/* check BSP_GLB_NONSECURE_GLB_OLOAD2 */ 
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) & 0x01 )  != 0x01);

	/* BSP_GLB_NONSECURE_GLB_OLOAD2 = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = 0x00;

	/* BSP_GLB_NONSECURE_GLB_HOST_INTR_STATUS = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS) = 0x00;  
		
	/* check status */
	status = *(volatile unsigned char*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x494);
			
	return status;
}

unsigned char read_msp(unsigned char msp_field, unsigned long *msp_data)
{
	unsigned char status;
	
	/* check BSP_GLB_NONSECURE_GLB_IRDY */
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_IRDY) & 0x02 )  != 0x02);
			
	/* version */	
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x300) = 0x10;
	/* seq */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x304) = 0x22;
	/* cont */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x308) = 0xabcdef00; 
	/* command ID to read MSP bit */ 	
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x30C) = 0xe455aa1b;  
	/*  command length in bytes.*/
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x310) = 0x789a0004; 
	/* read msp field */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x314) = msp_field;

	/* set BSP_GLB_NONSECURE_GLB_ILOAD2  */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_ILOAD2) = 0x01;

	/* check BSP_GLB_NONSECURE_GLB_OLOAD2 */ 
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) & 0x01 )  != 0x01);

	/* BSP_GLB_NONSECURE_GLB_OLOAD2 = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = 0x00;

	/* BSP_GLB_NONSECURE_GLB_HOST_INTR_STATUS = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS) = 0x00;  
		
	/* check status */
	status = *(volatile unsigned char*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x494);
	
	if ( status != 0x00 )
		return status;
		
	*msp_data = *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x498);
	
	return status;
}

unsigned char write_msp(
	unsigned char msp_field,
	unsigned long bit_length,
	unsigned long mask,	
	unsigned long msp_data
	)
{
	unsigned char status;
	
	status = send_otp_program_pattern();
	
	if(status)
	{
		status = send_otp_program_pattern();

		if(status)
			return status;	
	}
	
	/* check BSP_GLB_NONSECURE_GLB_IRDY */
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_IRDY) & 0x02 )  != 0x02);
			
	/* version */	
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x300) = 0x10;
	/* seq */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x304) = 0x22;
	/* cont */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x308) = 0xabcdef00; 
	/* command ID to write MSP bit */ 	
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x30C) = 0xe555aa1a;  
	/*  command length in bytes.*/
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x310) = 0x789a0014; 
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x314) = 0x12;
	/* read msp field */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x318) = msp_field;
	/* read msp field */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x31C) = bit_length;
	/* read msp field */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x320) = mask;
	/* read msp field */   
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x324) = msp_data;

	/* set BSP_GLB_NONSECURE_GLB_ILOAD2  */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_ILOAD2) = 0x01;

	/* check BSP_GLB_NONSECURE_GLB_OLOAD2 */ 
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) & 0x01 )  != 0x01);

	/* BSP_GLB_NONSECURE_GLB_OLOAD2 = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = 0x00;

	/* BSP_GLB_NONSECURE_GLB_HOST_INTR_STATUS = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS) = 0x00;  
		
	/* check status */
	return(*(volatile unsigned char*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE + 0x494));
}


void CmdGetLockStatus(void)
{
	unsigned long ulData;
	unsigned char ucStatus;
	
	/* check if EJTAG is disabled. */	
	ucStatus = read_msp(0x6, &ulData);
	if(ucStatus)
		printf("read failed status = %x\n", ucStatus );
	else
	{
		if(ulData)
			printf("EJTAG is disabled\n");
		else
		{
			printf("EJTAG is enabled\n");	
	
        	/* check if EJTAG is locked. */
        	ucStatus = read_msp(0x5, &ulData);
        	if(ucStatus)
        		printf("read failed status = %x\n", ucStatus );
        	else
        	{
        		if(ulData)
        			printf("EJTAG is locked\n");
        		else
        		{
        			printf("EJTAG is unlocked\n");	
        			
        			/* program ejtag lock bit. */
        			ucStatus = write_msp(0x5, 1, 1, 1);
        			if(ucStatus)
        				printf("progamming failure = %x\n", ucStatus);
        			else
        				printf("CrLockEnable was programmed\n");	
               				 				
        		}
        	}
        }
    }
	return;
}



int main(void)
{
	unsigned int size=	0x11000000;
	int fd;

	fd = open ("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0)
	{
		printf("cannot open /dev/mem.");
		return 1;
	} 

    /* We need to map register base for linux use space.  For other platform like
	   CFE, virtual_base will simply be 0xA0000000 */
	VIRTUAL_ADDRESS = (unsigned int)mmap(0,size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x00000000);
	
	CmdGetLockStatus();

	munmap((char *)VIRTUAL_ADDRESS, size); /*destroy map memory*/
	return 0;
}
