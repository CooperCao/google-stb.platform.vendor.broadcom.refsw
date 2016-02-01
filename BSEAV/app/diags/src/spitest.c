/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "test.h"

#ifdef DIAGS_SPI_TEST

#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "upg_handles.h"
#include "prompt.h"
#include "spitest.h"

#define MAX_SPI_TEST_SIZE	64

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif

#if __cplusplus
}
#endif

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/

#if __cplusplus
extern "C" {
#endif	   

#if __cplusplus
}
#endif

 /***********************************************************************
 *
 *  bcmSpiTest()
 * 
 *  SPI test
 *
 ***********************************************************************/
void bcmSpiTest (void)
{
	unsigned int		    command_id, choice;
	char 				    str[20];
	uint32_t			    i, val, subAddrBits;
	static uint8_t		    chipAddr=0;
	uint8_t				    subAddr8=0, wData[32], rData[32];
	uint16_t			    subAddr16=0;
	size_t				    length, numWriteBytes;
	BERR_Code			    error=BERR_SUCCESS;
	int					    newSpiChannel;
    static int              spiChannel=0;

    BREG_SPI_Handle  hSpiReg = bcmGetSpiRegHandle(spiChannel);

	while (1)
	{
		printf("\nSPI Tests (current channel=%d)\n\n", spiChannel);
		printf("0) Exit\n");
		printf("1) Change SPI channel\n");
		printf("2) Change default chip address\n");
		printf("3) SPI read\n");
		printf("4) SPI write\n");

		command_id = Prompt();
		
		switch(command_id)
		{
			case 0:
				return;

			case 1:
				printf("\nEnter the SPI channel (0 or 1): ");
				rdstr(str);
				sscanf(str, "%d", &newSpiChannel);
				if ((newSpiChannel==0) || (newSpiChannel==1))
				{
					spiChannel = newSpiChannel;
					hSpiReg = bcmGetSpiRegHandle(spiChannel);
				}
				else
				{
					printf("Invalid Spi channel.  Sticking with %d", spiChannel);
				}
				printf("\n");
				break;

			case 2:
				printf("\nEnter the chip address (in hex): ");
	            rdstr(str);
   		        sscanf(str, "%x", &val);
				chipAddr = (uint16_t)val;
				break;			

			case 3:
				if (!chipAddr)
				{
					printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
		            rdstr(str);
    		        sscanf(str, "%x", &val);
					chipAddr = (uint16_t)val;
				}
				else
				{
					printf("\nChip address = %02x\n", chipAddr);
				}
				wData[0] = chipAddr << 1;
				numWriteBytes = 1;

				printf("\nSub Address Bytes? (0, 1, or 2) ");
	            rdstr(str);
    	        sscanf(str, "%d", &choice);
				if ((choice == 1)  ||  (choice == 2))
				{			
					if (choice == 1)
						subAddrBits = 8;
					else if (choice == 2)
						subAddrBits = 16;
					else
						subAddrBits = 0;
					
					printf("\nEnter the sub address (in hex): ");
					if (subAddrBits == 8)
					{
			            rdstr(str);
    			        sscanf(str, "%x", &val);
						subAddr8 = (uint8_t)val;
					}
					else if (subAddrBits == 16)
					{
			            rdstr(str);
    			        sscanf(str, "%x", &val);
						subAddr16 = (uint16_t)val;
					}
				}
				else
				{
					subAddrBits = 0;
				}
				if (subAddrBits == 8)
				{
					wData[1] = subAddr8;
					numWriteBytes++;
				}
				else if (subAddrBits == 16)
				{
					wData[1] = (uint8_t)(subAddr16 >> 8);
					wData[2] = (uint8_t)(subAddr16 & 0xff);
					numWriteBytes += 2;
				}

				printf("\nEnter the number of bytes to read (maximum %d bytes): ", MAX_SPI_TEST_SIZE);
	            rdstr(str);
		        sscanf(str, "%d", &val);
		        length = (size_t)val;
		     	printf("\n");
		     	
				error = BREG_SPI_Read(hSpiReg, &wData[0], &rData[0], (numWriteBytes+length));
				if (error != BERR_SUCCESS)
					printf("SPI read fails %d!!!\n", error);
				else
				{
					i = numWriteBytes;
					while (length--)
						printf("\nValue read = %x", rData[i++]);
				}
				printf("\n");
				break;
				
			case 4:
				if (!chipAddr)
				{
					printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
					scanf("%x", &val);
					chipAddr = (uint16_t)val;
				}
				else
				{
					printf("\nChip address = %02x\n", chipAddr);
				}
				wData[0] = (chipAddr << 1) | 0x01;
				numWriteBytes = 1;

				printf("\nSub Address Bytes? (0, 1, or 2) ");
	            rdstr(str);
    	        sscanf(str, "%d", &choice);
				if ((choice == 1)  ||  (choice == 2))
				{			
					if (choice == 1)
						subAddrBits = 8;
					else if (choice == 2)
						subAddrBits = 16;
					else
						subAddrBits = 0;
					
					printf("\nEnter the sub address (in hex): ");
					if (subAddrBits == 8)
					{
			            rdstr(str);
    			        sscanf(str, "%x", &val);
						subAddr8 = (uint8_t)val;
					}
					else if (subAddrBits == 16)
					{
			            rdstr(str);
    			        sscanf(str, "%x", &val);
						subAddr16 = (uint16_t)val;
					}
				}
				else
				{
					subAddrBits = 0;
				}

				if (subAddrBits == 8)
				{
					wData[1] = subAddr8;
					numWriteBytes++;
				}
				else if (subAddrBits == 16)
				{
					wData[1] = (uint8_t)(subAddr16 >> 8);
					wData[2] = (uint8_t)(subAddr16 & 0xff);
					numWriteBytes += 2;
				}

				printf("\nEnter the number of bytes to write (maximum %d bytes): ", MAX_SPI_TEST_SIZE);
	            rdstr(str);
		        sscanf(str, "%d", &val);
		        length = (size_t)val;
		     	printf("\n");
		     	
				for (i=numWriteBytes; i < (numWriteBytes + length); i++)
				{
					printf("Enter the data you want to write: ");
		            rdstr(str);
			        sscanf(str, "%x", &val);
					wData[i] = (uint8_t)val;
				}
				
				error = BREG_SPI_Write(hSpiReg, &wData[0], (numWriteBytes+length));
				if (error != BERR_SUCCESS)
					printf("SPI write fails %d!!!\n", error);
				printf("\n");
				break;

			default:
				printf("Please enter a valid choice\n");
				break;
		}
	}

}

#else

void bcmSpiTest (void)
{
    printf("Not enabled\n");
}

#endif /* DIAGS_SPI_TEST */