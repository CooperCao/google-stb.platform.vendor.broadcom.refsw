/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#include "bstd.h"              /* standard types */

#include "bchp_mem_dma_0.h"
#include "bchp_common.h"
#include "bchp_bsp_cmdbuf.h"
#include "bchp_bsp_glb_control.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define DEBUG_MSG_DUMP
#define xprintf printf
unsigned int VIRTUAL_ADDRESS = 0xA0000000;
#define M2M_DESC_ADDR 0x0E000000 


#if(CFG_BIG_ENDIAN!=1)

static void Delay(void)
{
	int i;

	for ( i=0;i<10000000;i++);
}


static void endian_swap (unsigned int dataPhysicalAddress, unsigned int dataSize)
{
	unsigned long Addr = VIRTUAL_ADDRESS+M2M_DESC_ADDR;
	unsigned long CurrentDesc;
	unsigned long CurrentByte;
	unsigned long DmaStatus;

	if (dataSize==0 )
		return;

	xprintf("\n$$$$$ DMA Transfer $$$$$\n");

	/* use K1 address space for DMA */
	*(unsigned long *)(Addr) 	= dataPhysicalAddress;
	*(unsigned long *)(Addr+4) 	= dataPhysicalAddress;
	*(unsigned long *)(Addr+8) 	= 0xC0000000+dataSize;
	*(unsigned long *)(Addr+12) = 0xE000000|6; /* LE + byte swap */
	*(unsigned long *)(Addr+16) = 0x0;

	/* Update first descriptor address */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_FIRST_DESC) = 
		0x0E000000;
	
	/* Start from the first descriptor */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_CTRL) = 0;
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_CTRL) = 1;

	/* Read DMA status */
	CurrentDesc = *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_CUR_DESC);
	CurrentByte = *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_CUR_BYTE);
	while (1)
	{
		DmaStatus   = *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_STATUS);
		xprintf("DmaStatus = %x\n", (unsigned int)DmaStatus);
		if ( (DmaStatus & 3) != 1 )
			break;
	}
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_MEM_DMA_0_CTRL) = 0;

	xprintf("FirstDesc = %x\n", (unsigned int)Addr);
	xprintf("CurrentDesc = %x\n", (unsigned int)CurrentDesc);
	xprintf("CurrentByte = %x\n", (unsigned int)CurrentByte);
	xprintf("DmaStatus = %x\n", (unsigned int)DmaStatus);
}

#endif


/*****************************************************************************
Name:
send_command

Summary:
This function sends a BSP command.

Input:
unsigned long num_input - length of BSP command.
unsigned long *command_array - pointer to input command

Output:
N/A

Returns:
unsigned char - 0x0 if successful, otherwise fail.

See Also:

*****************************************************************************/
static unsigned char send_command(unsigned int num_input, unsigned int *command_array,
								  unsigned int num_output, unsigned int *command_array_out)
{
	unsigned int i;
	unsigned char status;

	/* check BSP_GLB_NONSECURE_GLB_IRDY */
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_IRDY) & 0x02 )  != 0x02);

	for (i=0; i<num_input; i++)
	{
		*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE+0x300+i*4) = command_array[i];
	}
#ifdef DEBUG_MSG_DUMP
    xprintf ("BSP input CMD buffer is [len-0x%x]:\n", (unsigned int)num_input);
    for (i=0; i<num_input; i++)
    {
        xprintf ("0x%03x:  %08lx\n", 0x180+i*4, *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE+0x300+i*4));
    }
#endif

	/* set BSP_GLB_NONSECURE_GLB_ILOAD2  */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_ILOAD2) = 0x01;

	/* check BSP_GLB_NONSECURE_GLB_OLOAD2 */ 
	while (  ( *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) & 0x01 )  != 0x01);

	/* BSP_GLB_NONSECURE_GLB_OLOAD2 = 0 */
	*(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_OLOAD2) = 0x00;

	 /* BSP_GLB_NONSECURE_GLB_HOST_INTR_STATUS = 0 */
	 *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_GLB_CONTROL_GLB_HOST_INTR_STATUS) = 0x00;   
	
	/* check status */
	status = *(volatile unsigned char*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE+0x494);
#ifdef DEBUG_MSG_DUMP
	xprintf ("BSP output CMD buffer is:\n");
	for (i=0; i<6; i++)
	{
		xprintf ("%08lx\n", *(volatile unsigned long*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE+0x480+i*4));
	}
#endif

	if ( num_output )
	{
		volatile unsigned int * pOut;
		unsigned int out_size = *(volatile unsigned char*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE+0x490);
		if ( (out_size-4) > (num_output*sizeof(unsigned int)) )
		{
#ifdef DEBUG_MSG_DUMP
			xprintf ("Out buffer is not big enough to hold all the output data\n");
			return 1;
#endif
		}
		pOut = (volatile unsigned int*) (VIRTUAL_ADDRESS+BCHP_PHYSICAL_OFFSET+BCHP_BSP_CMDBUF_DMEMi_ARRAY_BASE+0x498);
#ifdef DEBUG_MSG_DUMP
		xprintf ("Out buffer is :\n");
#endif
		for (i=0; i< (out_size-4)/(sizeof (unsigned int)); i++ )
		{
			command_array_out[i] = pOut[i];
#ifdef DEBUG_MSG_DUMP
			xprintf ("%08x\n", (unsigned int)command_array_out[i]);
#endif
		}
	}
	return status;				
}


static unsigned int gShaCommand [] =
{
	0x0bc10000,
	0x00000002,
	0xabcdef00,
	0xed55aa12,
	0x789a0014,
	0x00000000, /* VKL/KL NOT used for SHA */
	0x00010218, /* Use context 0 */
	0x00000014, /* Key length 0 */
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000, 
	0x00000000 
};


#define MAX_SHA_DIGEST_SIZE 32	/* SHA 256 */

typedef enum SHA_TYPE
{
	SHA_TYPE_160, 	/* SHA-1 */
	SHA_TYPE_224,
	SHA_TYPE_256,  	/* SHA-256 */
	SHA_TYPE_HMAC	/* HMAC SHA */
} SHA_TYPE;


typedef struct _SHA_Context 
{
	bool bFirstBlockProcessed;
	unsigned int 	opcode;
	unsigned int 	offsetDataLength;
	SHA_TYPE			hmac_sha_type;	/* TODO */
	unsigned char pxKey[32];			/* TODO */
	unsigned int 	sizeDigest;
	unsigned char digest[MAX_SHA_DIGEST_SIZE];
} SHA_Context;

static void SHA_Init (SHA_Context *pContext, SHA_TYPE shaType, unsigned char *key, unsigned int keySize)
{
	unsigned int i;
	memset (pContext, 0, sizeof (SHA_Context));
	pContext->bFirstBlockProcessed = false;
	switch (shaType)
	{
	case SHA_TYPE_160:
		pContext->hmac_sha_type = SHA_TYPE_160;
		pContext->sizeDigest = 20;
		pContext->opcode = 0x00010018;
		pContext->offsetDataLength = 13;
		break;
	case SHA_TYPE_224:
		pContext->hmac_sha_type = SHA_TYPE_224;
		pContext->sizeDigest = 28;
		pContext->opcode = 0x00010118;
		pContext->offsetDataLength = 15;
		break;
	case SHA_TYPE_HMAC:			/* TODO */
		pContext->hmac_sha_type = SHA_TYPE_HMAC;
		pContext->sizeDigest = 32;
		gShaCommand [5] = 0x1;
		pContext->opcode = 0x00010118;
		pContext->offsetDataLength = 16;
		for(i=0; i<keySize && i<32; i++)
			pContext->pxKey[i] = key[i];
		break;	
	case SHA_TYPE_256:
	default:
		pContext->hmac_sha_type = SHA_TYPE_256;
		pContext->sizeDigest = 32;
		pContext->opcode = 0x00010218;
		pContext->offsetDataLength = 16;
		break;
	}

}

static void SHA_Update (SHA_Context *pContext, unsigned int dataPhysicalAddress, unsigned int dataSize)
{
	unsigned int i;
	unsigned int bsp_out[96];

	gShaCommand [2] = 0xabcdef01;
	gShaCommand [4] = 0x789a0014+pContext->sizeDigest;
	gShaCommand [6] = pContext->opcode;
	gShaCommand [7] = pContext->sizeDigest;
	gShaCommand [pContext->offsetDataLength] = dataSize;
	gShaCommand [pContext->offsetDataLength+1] = dataPhysicalAddress;
	
	if(pContext->hmac_sha_type == SHA_TYPE_HMAC)
	{
		for(i=0; i<32; i+=4) {
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i]) << 24;
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i+1]) << 16;
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i+2]) << 8;
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i+3]);
		}
	}	
	
	send_command (sizeof(gShaCommand)/sizeof(unsigned int), gShaCommand, 8, bsp_out);

	if ( pContext->bFirstBlockProcessed== false )
		pContext->bFirstBlockProcessed = true;
}

static void SHA_Finish (SHA_Context *pContext, unsigned int dataPhysicalAddress, unsigned int dataSize)
{
	unsigned int i, bsp_out[96];
	unsigned int * pOut;

	gShaCommand [2] = ( pContext->bFirstBlockProcessed== false )? 0xabcdef00: 0xabcdef10;
	gShaCommand [4] = 0x789a0014+pContext->sizeDigest;
	gShaCommand [6] = pContext->opcode;
	gShaCommand [7] = pContext->sizeDigest;
	gShaCommand [pContext->offsetDataLength] = dataSize;
	gShaCommand [pContext->offsetDataLength+1] = dataPhysicalAddress;

	if(pContext->hmac_sha_type == SHA_TYPE_HMAC)
	{
		for(i=0; i<32; i+=4) {
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i]) << 24;
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i+1]) << 16;
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i+2]) << 8;
			gShaCommand [8+(i>>2)] |= (unsigned int)(pContext->pxKey[i+3]);
		}
	}	
	
	send_command (sizeof(gShaCommand)/sizeof(unsigned int), gShaCommand, 8, bsp_out);

	pOut = (unsigned int *)pContext->digest;
	for ( i=0; i<(pContext->sizeDigest/4);i++ )
	{
		pOut[i] = (bsp_out[i]<<24)|
					((bsp_out[i] & 0x0000FF00)<<8) |
					((bsp_out[i] & 0x00FF0000)>>8) |
					((bsp_out[i] & 0xFF000000)>>24);
	}

}

#define DATA_CHUNK_SIZE (128)
#define DATA_PHYSICAL_ADDESS 0x0F000000
#define SHA_DATA_SIZE 256

void mem_dump(unsigned char * BaseAddr, unsigned int size)
{
	unsigned int i;

	for (i=0; i<size; i++)
	{
		if ((i%16)==0)
			xprintf("\n");
		
		xprintf("%02x ", BaseAddr[i]);
		
	}
}

int sha_test (void)
{
	int fd;
	unsigned int i;
	unsigned int size=  0x11000000;
	/* unsigned int bsp_out[96];*/
	unsigned char * pIn;
	unsigned int runs, left_over;
	SHA_Context context, * pContext = &context;
	unsigned int dataPhysicalAddress;


	fd = open ("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0)
	{
		printf("cannot open /dev/mem.");
		return 1;
	}

	/* We need to map register base for linux use space.  For other platform like
	   CFE, virtual_base will simply be 0xA0000000 */
	VIRTUAL_ADDRESS = (unsigned int)mmap(0,size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x00000000);
	printf("Virtual Base is %08x\n", VIRTUAL_ADDRESS);
    
	pIn = (unsigned char *)(VIRTUAL_ADDRESS+DATA_PHYSICAL_ADDESS);
	for (i=0;i<SHA_DATA_SIZE;i++ )
	{
		pIn[i] = 0x00;
	}
	runs = SHA_DATA_SIZE/DATA_CHUNK_SIZE;
	left_over = SHA_DATA_SIZE%DATA_CHUNK_SIZE;
	dataPhysicalAddress = DATA_PHYSICAL_ADDESS;
	if ( (runs >0) && (left_over==0) )
	{
		runs--;
		left_over = DATA_CHUNK_SIZE;
	}

	SHA_Init (pContext, SHA_TYPE_160, NULL, 0);

	for (i=0; i<runs; i++ )
	{
#if(CFG_BIG_ENDIAN!=1)
		endian_swap (dataPhysicalAddress, DATA_CHUNK_SIZE);
#endif
		SHA_Update (pContext, dataPhysicalAddress, DATA_CHUNK_SIZE);
        dataPhysicalAddress += DATA_CHUNK_SIZE;
	}

#if(CFG_BIG_ENDIAN!=1)
	endian_swap (dataPhysicalAddress, left_over);
#endif
	SHA_Finish (pContext, dataPhysicalAddress, left_over);

	printf ("The SHA digest is: 0x");
	for ( i=0; i<pContext->sizeDigest;i++ )
	{
		printf("%02x", pContext->digest[i]);
	}

	printf("\n");

	munmap((char *)VIRTUAL_ADDRESS, size); /*destroy map memory*/
	return 0;
}


int hmac_sha_test (void)
{
	int fd;
	unsigned int i;
	unsigned int size=  0x11000000;
	/* unsigned int bsp_out[96]; */
	unsigned char * pIn;
	unsigned int runs, left_over;
	SHA_Context context, * pContext = &context;
	unsigned int dataPhysicalAddress;
	/*unsigned char pxKey[8] = {0x64, 0x5a, 0x48, 0x73, 0x40, 0xf4, 0xc7, 0xf0}; */
	unsigned char pxKey[8] = {0x73,0x48, 0x5a, 0x64,      0xf0, 0xc7, 0xf4,0x40  };
	unsigned char message[8] = {0x55, 0xf8, 0x0d, 0x13, 0x2e, 0x8b, 0x68, 0xeb};
	/* unsigned char refDigest[20] = {0x2f, 0x09, 0x28, 0xb4, 0xbb, 0x36, 0x5b, 0x4a, 0x59, 0x0d, 
																0x84, 0x96, 0x0a, 0x7c, 0xd0, 0x4f, 0xd2, 0xd8, 0x02, 0x21};
       */																
 

	fd = open ("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0)
	{
		printf("cannot open /dev/mem.");
		return 1;
	}

	/* We need to map register base for linux use space.  For other platform like
	   CFE, virtual_base will simply be 0xA0000000 */
	VIRTUAL_ADDRESS = (unsigned int)mmap(0,size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x00000000);
	printf("Virtual Base is %08x\n", VIRTUAL_ADDRESS);

  /* 
   * HMAC-SHA256
   */
	pIn = (unsigned char *)(VIRTUAL_ADDRESS+DATA_PHYSICAL_ADDESS);
	for (i=0;i<SHA_DATA_SIZE;i++ )
	{
		pIn[i] = 0x00;
	}

	
	/*runs = SHA_DATA_SIZE/DATA_CHUNK_SIZE; */
	runs = sizeof(message)/DATA_CHUNK_SIZE;
	
	/* left_over = SHA_DATA_SIZE%DATA_CHUNK_SIZE; */
	left_over = sizeof(message)%DATA_CHUNK_SIZE;

	dataPhysicalAddress = DATA_PHYSICAL_ADDESS;

	memcpy(pIn, message, sizeof(message));
	if ( (runs >0) && (left_over==0) )
	{
		runs--;
		left_over = DATA_CHUNK_SIZE;
	}

/*endian_swap (pxKey, sizeof(pxKey));	 debug only */

	SHA_Init (pContext, SHA_TYPE_HMAC, pxKey, sizeof(pxKey));

	for (i=0; i<runs; i++ )
	{
#if(CFG_BIG_ENDIAN!=1)
		endian_swap (dataPhysicalAddress, DATA_CHUNK_SIZE);
#endif
		SHA_Update (pContext, dataPhysicalAddress, DATA_CHUNK_SIZE);
        dataPhysicalAddress += DATA_CHUNK_SIZE;
	}

#if(CFG_BIG_ENDIAN!=1)
	endian_swap (dataPhysicalAddress, left_over);
#endif
	SHA_Finish (pContext, dataPhysicalAddress, left_over);

	printf ("The SHA digest is: 0x");
	for ( i=0; i<pContext->sizeDigest;i++ )
	{
		printf("%02x", pContext->digest[i]);
	}

	printf("\n");

	munmap((char *)VIRTUAL_ADDRESS, size); /*destroy map memory*/
	return 0;
}

int main (void)
{
	char select;
	
	printf("Selection:\n");
	printf("1: SHA test\n");
	printf("2: HMAC_SHA test\n");
	printf("q: quit\n\n");
	scanf("%s", &select);
	Delay();
	
	if(select == '1')
		sha_test();
	else if(select == '2')
		hmac_sha_test();
	else if(select == 'q')
		return 0;
	
	return 0;
}
