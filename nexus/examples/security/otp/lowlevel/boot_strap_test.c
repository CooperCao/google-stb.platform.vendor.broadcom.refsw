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


#include "bstd.h"              /* standard types */

#include "bchp_common.h"
#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_glb_control.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_jtag_otp.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>


#define DEBUG_MSG_DUMP					
#define xprintf printf







#define ADDR	736;



void waitForProdOtpCmdComplete(unsigned int VIRTUAL_ADDRESS)
{
    uint32_t status_reg;
	int i;
    status_reg = 0;
    while((status_reg & 0x1) == 0x0)
    {
	/*		cfe_msleep(10);*/
		for(i=0;i<100000;i++)
			{
				/*empty loop to simulate delay*/
			}
		status_reg = *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_STATUS_1);
    }    

    return;
}


void enableCustProdOtpWrite(unsigned int VIRTUAL_ADDRESS)
{

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x0000000f;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x00000004;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x00000008;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x0000000D;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);    

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;
    
    return;
    
}

void enableProdOtpWrite(unsigned int VIRTUAL_ADDRESS)
{

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x0000000f;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x00000004;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x00000008;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = 0x0000000D;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00200003;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);    

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00000000;
    
    return;
    
}




long readRefOk(unsigned int VIRTUAL_ADDRESS, long addr)
{
    long refOk;
    long general_ctrl_1;

	general_ctrl_1 = *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_1);

    general_ctrl_1 |= BCHP_JTAG_OTP_GENERAL_CTRL_1_jtag_otp_cpu_mode_MASK;
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_1) = general_ctrl_1;
    
    enableProdOtpWrite(VIRTUAL_ADDRESS);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = addr;
    
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x0;
        
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x20A00001;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);
    
	refOk = *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_STATUS_1);
    
    return refOk;
    
}


unsigned long readBootStrap(unsigned int VIRTUAL_ADDRESS, long Row23_ADDR)
{
	
	long refOK;
	unsigned long strapSetting = 0;

	refOK = readRefOk(VIRTUAL_ADDRESS, Row23_ADDR);
	
	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0;

    *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00F00001;
    
    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);
    
	strapSetting = *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_STATUS_0);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x0;

	return strapSetting;

    
}

void programBootStrap(unsigned int VIRTUAL_ADDRESS,long data, long Row23_ADDR)
{
	long general_ctrl_1;

	general_ctrl_1 = *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_1) ;
	  
    general_ctrl_1 |= 0x1;
    *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_1) = general_ctrl_1;

    enableCustProdOtpWrite(VIRTUAL_ADDRESS);

    
    *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_2) = data;

    *(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_3) = Row23_ADDR;


   	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x00F00017;


    waitForProdOtpCmdComplete(VIRTUAL_ADDRESS);

	*(volatile long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_JTAG_OTP_GENERAL_CTRL_0) = 0x0;

}



int main (void)
{
	int fd;
	unsigned int size=  0x11000000;
	unsigned int virtual_base;
	unsigned long strap_setting = 0;

	fd = open ("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0)
	{
		printf("cannot open /dev/mem.");
		return 1;
	}

	/* We need to map register base for linux use space.  For other platform like
	   CFE, virtual_base will simply be 0xB0000000 */

	virtual_base = (unsigned int)mmap(0,size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x00000000);

	strap_setting = readBootStrap(virtual_base,736);	/*read boot strap: virtual_base, address for boot strap (736). Current value will be returned*/

	printf("Current strap Setting is: %08x\n", (unsigned int)strap_setting);
				
	

	munmap((char *)virtual_base, size); /*destroy map memory*/
	return 0;
}
