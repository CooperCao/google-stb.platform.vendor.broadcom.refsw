/***************************************************************************
 *     Copyright (c) 2002-2009, Broadcom Corporation
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

#include "si.h"
#include "si_os.h"
#include "si_dbg.h"
#include "si_util.h"

unsigned long SI_Construct_Data( unsigned char * rawdat, unsigned long idx, unsigned long num, unsigned long shift, unsigned long mask)
{
	unsigned long data = 0;
	long i;

	if (num > 4)
	{
		SI_DBG_PRINT(E_SI_ERR_MSG, ("Error in SI_Construct_Data, the number of bytes has to be <= 4!!!\n"));
		return 0;
	}

	for (i=0; i<num; i++)
		data |= ( ((unsigned long)rawdat[i+idx])<<((num-1-i)*8) );
	data >>= shift;

	return (data&mask);
}

SI_RET_CODE SI_CRC32_Check ( unsigned char * data, unsigned short length)
{
	unsigned long reg = 0xffffffff; /* Annex B of ISO/IEC 13818-1, init state all 1's */
	unsigned long add_mask = 0x04c11db6; /* generation poly coefficient except for bit0. */
	unsigned short i;
	short bit;
	unsigned char input, z31;

	for (i=0; i<length; i++)
	{
		for (bit=7; bit>=0; bit--)
		{
			z31 = (reg&0x80000000)?0x1:0x0;
			reg <<= 1;
			input = (data[i] & (0x0001<<bit))?0x01:0x00;
			if (z31 ^ input)
			{
				reg ^= add_mask;
				reg |= 1;
			}
		}
	}

	if (reg)
		return SI_CRC_ERROR;

	return SI_SUCCESS;
}

void SI_Init_Section_Mask(unsigned long *mask, unsigned char last_section_number)
{
	unsigned char index, bit, i;

	index = last_section_number/32;
	bit = last_section_number % 32;

	for (i=0; i<index; i++)
		mask[i] = 0;
	for (i=index+1; i<8; i++)
		mask[i] = 0xffffffff;
	mask[index] = (0xffffffff<<(bit+1));
}

unsigned long SI_Chk_Section_mask(unsigned long *mask, unsigned char section_number)
{
	unsigned char index, bit;

	index = section_number/32;
	bit = section_number % 32;

	return (mask[index]&(1<<bit));
}

void SI_Set_Section_mask(unsigned long *mask, unsigned char section_number)
{
	unsigned char index, bit;

	index = section_number/32;
	bit = section_number % 32;

	mask[index] |= (1<<bit);
}

