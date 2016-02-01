/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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
#include <stdio.h>
#include <string.h>

unsigned long m[32];
unsigned long rand_num;

void init_rand(void)
{
	int i;
    for( i=0; i<32; i++ )  m[i] = 1L<<i;
	rand_num=0;
}

unsigned long get_rand(void)
{
    int a, b, c, d = 0;
   
    a = (m[31] & rand_num) ? 1 : 0;
    b = (m[21] & rand_num) ? 1 : 0;
    c = (m[ 1] & rand_num) ? 1 : 0;
    d = (m[ 0] & rand_num) ? 1 : 0;
    rand_num = (rand_num << 1) | (1^(a ^ b ^ c ^ d));
	return rand_num;
}
