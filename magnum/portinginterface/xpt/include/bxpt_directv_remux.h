/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * Porting interface code for the DirecTV portions of the packet remultiplexor. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*= Module Overview *********************************************************
This module supports the DirecTV portions of the packet remux block. The 
function calls allow the user to get or set the DirecTV fields in the 
remux. The calls are otherwise identical to the MPEG remux API.
***************************************************************************/

#ifndef BXPT_DIRECTV_REMUX_H__
#define BXPT_DIRECTV_REMUX_H__

#include "bxpt.h"
#include "bxpt_remux.h"

/***************************************************************************
Summary:
Defines the types of packets that the remux supports. 
****************************************************************************/
typedef enum BXPT_RemuxMode
{
	BXPT_RemuxMode_eDirecTv,
	BXPT_RemuxMode_eMpeg
}
BXPT_RemuxMode;

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Enable or disable DirecTV mode operation in the remux channel. 

Description:
This call should be used to put a remux channel into DirecTV mode, or to
restore MPEG mode after entering DirecTV mode.

Returns:
    BERR_SUCCESS                - Call completed successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_DirecTvRemux_SetMode( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	BXPT_RemuxMode Mode			/* [in] Selects the mode. */
	);

#if BXPT_HAS_PSUB_IN_REMUX
/***************************************************************************
Summary:
Enable or disable HD field matching in the Packet Sub block. 

Description:
Require packet substitution logic in the PacketSub block to also match the 
HD field in the transport packet to a given value before allowing a 
substitution of the payload.

Returns:
    BERR_SUCCESS                - Call completed successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Remux_PsubMatchHdField( 
	BXPT_Remux_Handle hRmx,	/* [in] Handle for the remux channel */
	int WhichTable,		/* [in] The remux packet sub to set. */
	bool MatchHd, 		  /* [in] Enable or disable HD match requirement */
	uint8_t HdCompValue	  /* [in] Value HD field must match. */
	);
#endif
#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_DIRECTV_REMUX_H__ */

/* end of file */


