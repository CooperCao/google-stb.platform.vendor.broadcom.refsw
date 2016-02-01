/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Module Description: Portting interface for the Special Address Mode PSI
 * filters.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
  
#ifndef BXPT_SAM_H
#define BXPT_SAM_H

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif

/*=************************ Module Overview ********************************
Overview:

This module is the porting interface for the Special Addressing Mode of the
PSI filters.

Note that on some architectures, not all PID channels support SAM filtering.
For the 7038, only the first 32 PID channels can use the SAM filters.

If SAM filtering is done in conjunction with normal PSI section filtering,
the sections must pass BOTH types of filtering in order to be accepted (and
therefore routed to the DRAM buffers for host access).
***************************************************************************/

/***************************************************************************
Summary:
Used to define the sizes of the various SAM addreses, in bytes.
****************************************************************************/
#define BXPT_SAM_40BIT_ADDR_SIZE	( 5 )
#define BXPT_M24_ADDR_SIZE			( 3 )
#define BXPT_M16_ADDR_SIZE			( 2 )

/***************************************************************************
Summary:
Multicast 16 address filters.

Description:
This structure contains the 4 multicast-16 address filters for each PID 
channel 0-31. These address filters are used in the PSI SAM filter when 
M16En is true for a PID channel.

The address bytes are stored in the arrays MSB first.
***************************************************************************/
typedef struct BXPT_SAM_M16Addresses
{
	uint8_t Address0[ BXPT_M16_ADDR_SIZE ];
	uint8_t Address1[ BXPT_M16_ADDR_SIZE ];
	uint8_t Address2[ BXPT_M16_ADDR_SIZE ];
	uint8_t Address3[ BXPT_M16_ADDR_SIZE ];
}
BXPT_SAM_M16Addresses;

/***************************************************************************
Summary:
40-bit address filters, with wildcard configuration.

Description:
This structure contains the 40 address filter used by the Type 0/1 Network 
address and Type 0/1 M40 address filters. Also included are the wildcard
configurations.

The address bytes are stored in the arrays MSB first.
***************************************************************************/
typedef struct BXPT_SAM_40BitAddr
{
	bool WildCardByteMode;		/* Byte mode if true, nibble mode if false. */
	bool WildCardEnable;		/* Enable wildard use. */
	uint8_t AddressWildcard;
	uint8_t Address[ BXPT_SAM_40BIT_ADDR_SIZE ];	
}
BXPT_SAM_40BitAddr;

/***************************************************************************
Summary:
Special Addressing Mode configuration.

Description:
Address filters for the Physical, Network, M24, and M40 addresses. These
values are global to all PID channels that use enable the respective SAM 
filter. The IdRejectHiLimit and IdRejectLowLimit are also global.

The address bytes are stored in the arrays MSB first.
***************************************************************************/
typedef struct BXPT_SAMAddresses
{
	/* Singlecast or physical address. */
	uint8_t PhysicalAddr[ BXPT_SAM_40BIT_ADDR_SIZE ];

	/* Type 0 network address. Global for all PID channels that enable it. */
	BXPT_SAM_40BitAddr Type0NetworkAddr;

	/* Type 1 network address. Global for all PID channels that enable it.  */
	BXPT_SAM_40BitAddr Type1NetworkAddr;

	/* Type 0 multicast 40 address. Global for all PID channels that enable it.  */
	BXPT_SAM_40BitAddr Type0M40Addr;

	/* Type 1 multicast 40 address. Global for all PID channels that enable it.  */
	BXPT_SAM_40BitAddr Type1M40Addr;

	/* Multicast 24 address. Global for all PID channels that enable it. */
	uint8_t M24Addr[ BXPT_M24_ADDR_SIZE ];	 

	/* Defines the upper and lower limits when filtering by Table ID. */
	uint8_t IdRejectHiLimit;
	uint8_t IdRejectLowLimit;
}
BXPT_SAMAddresses;

/***************************************************************************
Summary:
Special Addressing Mode enables.

Description:
Each PID channel may have one or more of the different SAM filters enabled
at any one time. For convenience, the enables for each mode are located in
a common structure, which is passed to the BXPT_SAM_DoFiltering() function.
***************************************************************************/
typedef struct BXPT_SAM_Enables
{
	bool PhysicalEn;
	bool NetworkEn;
	bool M40En;
	bool M24En;
	bool M16En;
	bool IdRejectEn;

	/* Note that the NOT function is applied to ALL of the filters enabled above. */
	bool SamNotEn;
}
BXPT_SAM_Enables;

/***************************************************************************
Summary:
Return the multicast-16 address filters.

Description:
The multicast-16 filters are a set of 4 16-bit address filters. Each message 
buffer that supports SAM filtering has 1 such set. The function returns the
filter set for the given buffer.

Returns:
    BERR_SUCCESS                - Data retrieved.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_SAM_SetM16Addresss
****************************************************************************/
BERR_Code BXPT_SAM_GetM16Addresss(
	BXPT_Handle hXpt,
	unsigned int MessageBufferNum,
	BXPT_SAM_M16Addresses *Addresses
	);

/***************************************************************************
Summary:
Set the multicast-16 address filters.

Description:
The multicast-16 filters are a set of 4 16-bit address filters. Each message 
buffer that supports SAM filtering has 1 such set. The function loads the
filter set for the given buffer.

Returns:
    BERR_SUCCESS                - Filters loaded.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_SAM_GetM16Addresss
****************************************************************************/
BERR_Code BXPT_SAM_SetM16Addresss(
	BXPT_Handle hXpt,
	unsigned int MessageBufferNum,
	const BXPT_SAM_M16Addresses *Addresses
	);

/***************************************************************************
Summary:
Return the SAM global address filters.

Description:
The filters for the Physical, Network, M24, and M40 addresses are global, for
all buffers that enable them. This function returns the current filter 
settings from hardware.

Returns:
    BERR_SUCCESS                - Filters retrieved from hardware.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_SAM_SetAddresses
****************************************************************************/
BERR_Code BXPT_SAM_GetAddresses(
	BXPT_Handle hXpt,
	BXPT_SAMAddresses *Addresses
	);

/***************************************************************************
Summary:
Set the SAM global address filters.

Description:
The filters for the Physical, Network, M24, and M40 addresses are global, for
all buffers that enable them. This function loads the given filter 
settings into hardware.

Returns:
    BERR_SUCCESS                - Filters loaded.
    BERR_INVALID_PARAMETER      - Bad input parameter
 
See Also:
BXPT_SAM_SetAddresses
****************************************************************************/
BERR_Code BXPT_SAM_SetAddresses(
	BXPT_Handle hXpt,
	const BXPT_SAMAddresses *Addresses
	);

/***************************************************************************
Summary:
Enable or disable SAM filtering.

Description:
SAM filtering can is enabled or disabled on a per-buffer basis. This 
function enables or disables the SAM filters as specified in the Enables 
structure. 

Returns:
    BERR_SUCCESS                - Enable succesful.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_SAM_DoFiltering(
	BXPT_Handle hXpt,
	unsigned int MessageBufferNum,
	BXPT_SAM_Enables *Enables
	);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_SAM_H */

/* end of file */





