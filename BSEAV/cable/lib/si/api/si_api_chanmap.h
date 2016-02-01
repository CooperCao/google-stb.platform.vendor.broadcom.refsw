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


/*= Module Overview *********************************************************
<verbatim>

Overview
In a POD base open cable DTV or STB system, all the channel map information
comes from either an OOB channel through POD or a DSG tunnel through host to
POD. The channel map can be parsed from a combination of NIT, NTT and SVCT or
LVCT as defined in ANSI/SCTE65 (DVS234). Information about the channel number,
physical channel frequency, modulation mode, symbol rate,program number and etc
is all included in the channel map defined in this API. The upper level
application can use this information to acquire the physical channel and lock
on the transport stream.


Design
Currently the Channel Map API only support one API function call for the upper
level applications to get the current channel map.


Usage
The usage of Channel Map API involves the upper level application periodically
calling the get channel map API functions to get complete up to date channel map.


Interrupt Requirements
None.


Sample Code
None.

</verbatim>
***************************************************************************/

#ifndef SI_API_CHANMAP_H
#define SI_API_CHANMAP_H

#include "channellist.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
	This function tries to get the channel map from SI data received from
	either OOB channel or DSG tunnel.

Description:
	This function is responsible for trying to get the channel map from POD
	SW stack. Since per specified by ANSI/SCTE 65, it may take some time before
	all the channel map information can arrive and it can change without notice
	the upper level application is responsible for calling this function
	periodically to keep the channel map complete and up to date.

Returns:
	0: when the function returns successfully.
	-1: when the function fails.

See Also:


****************************************************************************/
int SCTE_Api_Get_Chan_Map(channel_list_t *list, unsigned int *num_ch);

/***************************************************************************
Summary:
	This function resets SI module ans start over for getting new channel map

Description:
	This function is responsible to reset all SI modules and start over for getting
	new channel map

Returns:
	N/A

See Also:

****************************************************************************/
void SCTE_Api_Clear_Chan_Map(void);

/***************************************************************************
Summary:
	This function checks if there is a new version complete channel map
	available.

Description:
	This function checks if there is a new version complete channel map
	available. CDS, VCM and MMS tablea must be completely recieved and processed.
	NTT table is not required.

Returns:
	0: available
	-1: not available
	1: in process

See Also:

****************************************************************************/

int SCTE_Api_Check_Chan_Map();

/***************************************************************************
Summary:
	This function tries to get the channel map from LVCT, TVCT or CVCT

Description:
	This function is responsible for trying to get the channel map

Returns:
	0: when the function returns successfully.
	-1: when the function fails.
See Also:


****************************************************************************/
int PSIP_Api_Get_Chan_Map(channel_list_t *list, unsigned int *num_ch, unsigned int freq);

/***************************************************************************
Summary:
	This function resets SI module ans start over for getting new channel map

Description:
	This function is responsible to reset all SI modules and start over for getting
	new channel map

Returns:
	N/A

See Also:

****************************************************************************/
void PSIP_Api_Clear_Chan_Map(void);

/***************************************************************************
Summary:
	This function checks if there is a new version complete channel map
	available.

Description:
	This function checks if there is a new version complete channel map
	available.

Returns:
	0: available
	-1: not available
	1: in process

See Also:

****************************************************************************/

int PSIP_Api_Check_Chan_Map();

#ifdef __cplusplus
}
#endif


#endif
