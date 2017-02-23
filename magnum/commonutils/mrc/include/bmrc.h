/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ***************************************************************************/

#ifndef BMRC_H
#define BMRC_H

#include "berr_ids.h"            /* Error codes */
#include "bint.h"                /* Returning the interrupt ID of slot */
#include "bchp_memc_clients.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
	List of errors unique to MRC
****************************************************************************/
#define BMRC_CHECKER_ERR_ENABLED_CANT_SET        BERR_MAKE_CODE(BERR_MRC_ID, 0)
#define BMRC_CHECKER_ERR_ALL_USED                BERR_MAKE_CODE(BERR_MRC_ID, 1)
#define BMRC_CHECKER_ERR_NO_CALLBACK_SET         BERR_MAKE_CODE(BERR_MRC_ID, 2)

/***************************************************************************
Summary:
	Memory checker access types.

Description:
	The different types of memory accesses handled by the memory range
	checkers.

See Also:
	BMRC_Checker_SetAccessCheck
	BMRC_Checker_SetClient
****************************************************************************/
typedef enum
{
	BMRC_AccessType_eNone = 0x00,
	BMRC_AccessType_eRead = 0x01,
	BMRC_AccessType_eWrite = 0x02,
	BMRC_AccessType_eBoth =  BMRC_AccessType_eRead | BMRC_AccessType_eWrite
} BMRC_AccessType;


/***************************************************************************
Summary:
	List of memory ranger checker clients.

Description:
	This is the enumerated list of clients that the Memory Range Checker
	module checks and grants memory access rights to.

See Also:
	BMRC_Checker_SetClient
****************************************************************************/
typedef enum BCHP_MemcClient BMRC_Client;
#define BMRC_Client_eMaxCount BCHP_MemcClient_eMax
#define BMRC_Client_eInvalid BCHP_MemcClient_eMax


/***************************************************************************
Summary:
	The handle for the MRC module.

Description:
	This is the main module handle required in order to create checkers.

See Also:
	BMRC_Open
****************************************************************************/
typedef struct BMRC_P_Context *BMRC_Handle;


/***************************************************************************
Summary:
	The handle for memory access checkers.

Description:
	Handle for memory access checkers returned by BMRC_Checker_Create.

See Also:
	BMRC_Checker_Create
****************************************************************************/
typedef struct BMRC_P_CheckerContext *BMRC_Checker_Handle;


/***************************************************************************
Summary:
	This structure contains the Memory Range Checker module settings.

Description:
	Configures the Memory Range Checker module when it is opened.

See Also:
	BMRC_Open
***************************************************************************/
typedef struct BMRC_Settings
{
	uint16_t usMemcId; /* Selects which memc to use */
} BMRC_Settings;


/***************************************************************************
Summary:
	This structure contains a client's information.

Description:
	This information is returned by BMRC_Checker_GetClientInfo.
	The translation from eClient to usClientId is done in BMRC_P_astClientTbl[] which is in bmrc_clienttable_priv.c.
***************************************************************************/
typedef struct BMRC_ClientInfo
{
	const char     *pchClientName;
	BMRC_Client     eClient;    /* SW client enum */
	uint16_t        usClientId; /* HW client bit position (values 0..127 spanning 4 32-bit registers) */
} BMRC_ClientInfo;


/***************************************************************************
Summary:
	This structure contains the checker and access violation information
	returned to a registered callback.

Description:
	BMRC_CheckerInfo provides the checker id, memory range, address that
	violated the checker, request type, nmbx id, and the client responsible
	for the violation.
***************************************************************************/
typedef struct BMRC_CheckerInfo
{
	uint16_t        usMemcId;     /* memory controller id */
	uint16_t        usCheckerId;  /* checker id */
	BSTD_DeviceOffset ulStart;      /* memory range start */
	uint64_t          ulSize;       /* memory range size */
	BSTD_DeviceOffset ulAddress;    /* start address that violated the range checker */
	BSTD_DeviceOffset ulAddressEnd; /* end address that violated the range checker */
#if (BCHP_CHIP != 7440) && (BCHP_CHIP != 7601) && (BCHP_CHIP != 7635) && (BCHP_CHIP != 7630) && (BCHP_CHIP != 7640)
	uint32_t        ulReqType;    /* req type */
	uint32_t        ulNmbxId;     /* nmbx id */
	uint32_t        ulNmbx;       /* nmbx */
#else
	uint32_t        ulLength;
	uint32_t        ulMode;
	uint32_t        ulWrite;
#endif
	uint16_t        usClientId;    /* client that violated the range checker */
	const char     *pchClientName; /* client name */
	bool            bExclusive;    /* exclusive mode */
} BMRC_CheckerInfo;


/***************************************************************************
Summary:
	Prototype of a memory range checker callback function.

Description:
	Upper level applications register callbacks to a checker that will be
	executed when a checker violation occurs.

See Also:
	BMRC_Checker_SetCallback
**************************************************************************/
typedef void (*BMRC_CallbackFunc_isr)( void *pvData1, int iData2, BMRC_CheckerInfo *pInfo);


/***************************************************************************
Summary:
	Gets the default settings.

Description:
	Fills the default settings structure with default values.

Returns:
	BERR_SUCCESS - Handle was successfully closed.
	BERR_INVALID_PARAMETER - Handle was invalid.

See Also:
	BMRC_Open
**************************************************************************/
BERR_Code BMRC_GetDefaultSettings
	( BMRC_Settings *pDefSettings  /* [out] Default settings structure to fill */
	);


/***************************************************************************
Summary:
	Opens the MRC module.

Description:
	Opens the Memory Range Checker module and creates its context.

Returns:
	BERR_SUCCESS - Handle was successfully created.
	BERR_OUT_OF_SYSTEM_MEMORY - Unable to allocate memory for the handle.

See Also:
	BMRC_Close
**************************************************************************/
BERR_Code BMRC_Open
	( BMRC_Handle                     *phMrc,        /* [out] MRC handle to be returned */
	  BREG_Handle                      hRegister,    /* [in] Register access handle */
	  BINT_Handle                      hInterrupt,   /* [in] Interrupt handle */
	  const BMRC_Settings             *pDefSettings  /* [in] Default settings */
	);


/***************************************************************************
Summary:
	Closes the MRC module.

Description:
	Closes the Memory Range Checker module and its context.

Returns:
	BERR_SUCCESS - Handle was successfully closed.
	BERR_INVALID_PARAMETER - Handle was invalid.

See Also:
	BMRC_Open
**************************************************************************/
BERR_Code BMRC_Close
	( BMRC_Handle hMrc /* [in] MRC handle to close */
	);


/***************************************************************************
Summary:
	Return settings passed into BMRC_Open
**************************************************************************/
void BMRC_GetSettings
	( BMRC_Handle hMrc,                     /* [in] MRC handle to close */
	  BMRC_Settings             *pSettings  /* [out] */
	);

/***************************************************************************
Summary:
	Gets maximum number of MRC checkers.

Description:
	Gets maximum number of checkers available to this MRC isntance, depending
	on its associated memory controller.

Returns:
	BERR_SUCCESS - Handle was successfully closed.
	BERR_INVALID_PARAMETER - Handle was invalid.

See Also:
**************************************************************************/
BERR_Code BMRC_GetMaxCheckers
	( BMRC_Handle hMrc,         /* [in] MRC handle */
	  uint32_t *pulMaxCheckers  /* [out] Maximum number of checkers for this MRC */
	);


/***************************************************************************
Summary:
	Creates a memory range access Checker.

Description:
	Creates a checker that can be used to check memory access to a certain
	range in memory, with the ability to check for specific types of
	violations by specific clients based on how the checker is configured.

Returns:
	BERR_SUCCESS - Checker handle was successfully created.
	BERR_INVALID_PARAMETER - Mrc handle was invalid.
	BMRC_CHECKER_ERR_ALL_USED - Maximum number of checkers reached and no
	                            additional checkers can be created.


See Also:
	BMRC_Checker_Destroy
**************************************************************************/
BERR_Code BMRC_Checker_Create
	( BMRC_Handle hMrc,               /* [in] MRC Module Handle */
	  BMRC_Checker_Handle *phChecker  /* [out] Checker handle to be returned */
	);


/***************************************************************************
Summary:
	Destroys a memory range access Checker.

Description:
	Disables and destroys an existing checker and cleans up its resources.

Returns:
	BERR_SUCCESS - Checker handle was successfully destroyed.
	BERR_INVALID_PARAMETER - Checker handle was invalid.

See Also:
	BMRC_Checker_Create
**************************************************************************/
BERR_Code BMRC_Checker_Destroy
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle to be destroyed */
	);


/***************************************************************************
Summary:
	Sets a checker's memory access range.

Description:
	Configures the memory range that a checker is to check, based on a
	given memory address and range size.  The function cannot be called
	when a checker is already enabled.

Returns:
	BERR_SUCCESS - Checker range was successfully set.
	BERR_INVALID_PARAMETER - Checker handle was invalid, ulStart address
							 or ulSize in bytes aren't 8 byte aligned.
	BMRC_CHECKER_ERR_ENABLED_CANT_SET - Checker already enabled, cannot
	                                    be set.

See Also:
**************************************************************************/
BERR_Code BMRC_Checker_SetRange
( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
  BSTD_DeviceOffset ulStart,              /* [in] Memory range start address */
  uint64_t ulSize                /* [in] Memory range size */
  );


/***************************************************************************
Summary:
	Configures what types of accesses a checker should check for.

Description:
	This function sets up what sorts of accesses a checker checks.

	Setting eAccessType to BMRC_AccessType_eRead causes violations to occur
	only when clients without read rights attempt to read from the checker's
	range.

	Setting eAccessType to BMRC_AccessType_eWrite causes violations to occur
	only when clients without write rights attempt to write from the
	checker's range.

	Setting eAccessType to BMRC_AccessType_eBoth causes violations when
	either unauthorized reads or writes occur.  The function cannot be
	called when a checker is already enabled.

Returns:
	BERR_SUCCESS - The access check type was successfully set.
	BERR_INVALID_PARAMETER - Checker handle was invalid, or
	                         BMRC_Access_eNone was used as the
	                         access type to check.
	BMRC_CHECKER_ERR_ENABLED_CANT_SET - Checker already enabled, cannot
	                                    be set.

See Also:
**************************************************************************/
BERR_Code BMRC_Checker_SetAccessCheck
	( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
	  BMRC_AccessType eAccessType    /* [in] Access type to check */
	);


/***************************************************************************
Summary:
	Configures what types of accesses are blocked in the event of a
	violation.

Description:
	This function blocks read and/or write access to a checker's range when
	a violation does occur.

	Setting eBlockType to BMRC_AccessType_eRead blocks further read
	accesses

	Setting eBlockType to BMRC_AccessType_eWrite blocks further write
	accesses

	Setting eBlockType to BMRC_AccessType_eBoth blocks both types of
	accesses when a violation occurs.

	The function cannot be called when a checker is already enabled.

Returns:
	BERR_SUCCESS - The block type was successfully set.
	BERR_INVALID_PARAMETER - Checker handle was invalid.
	BMRC_CHECKER_ERR_ENABLED_CANT_SET - Checker already enabled, cannot
	                                    be set.

See Also:
**************************************************************************/
BERR_Code BMRC_Checker_SetBlock
	( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
	  BMRC_AccessType eBlockType     /* [in] Access type to block on violations*/
	);


/***************************************************************************
Summary:
	Sets a checker's exclusive mode.

Description:
	When a checker is configured for exclusive mode, clients with read
	access to the checker's memory range can only read from that range, and
	clients	with write access can only write to that range.  Accesses
	to other memory ranges by these clients will cause a violation.	The
	function cannot be called when a checker is already enabled.

Returns:
	BERR_SUCCESS - The checker's exlusive mode was successfully set.
	BERR_INVALID_PARAMETER - Checker handle was invalid.
	BMRC_CHECKER_ERR_ENABLED_CANT_SET - Checker already enabled, cannot
	                                    be set.

See Also:
**************************************************************************/
BERR_Code BMRC_Checker_SetExclusive
	( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
	  bool bExclusive                /* [in] Enable/disable exclusive mode */
	);


/***************************************************************************
Summary:
	Configures a client's access rights to the checker's memory range.

Description:
	This function sets a client's permissions to access the checker's memory
	range.  A client can be given read rights, write rights, both, or none
	by setting eAccessType to BMRC_AccessType_eRead, BMRC_AccessType_eWrite,
	BMRC_AccessType_eBoth, or BMRC_AccessType_eNone.

Returns:
	BERR_SUCCESS - Client was successfully configured.
	BERR_INVALID_PARAMETER - Checker handle was invalid.
	BMRC_CHECKER_ERR_ENABLED_CANT_SET - Checker already enabled, cannot
	                                    be set.

See Also:
	BMRC_Checker_SetAccess
**************************************************************************/
BERR_Code BMRC_Checker_SetClient
	( BMRC_Checker_Handle hChecker,  /* [in] Checker handle */
	  BMRC_Client eClient,           /* [in] The client to configure */
	  BMRC_AccessType eAccessType    /* [in] The client's access rights */
	);


/***************************************************************************
Summary:
	Enables a client's access checking.

Description:
	This function enables a client's access checking and will produce
	violations when they occur.

Returns:
	BERR_SUCCESS - Checker was successfully enabled.
	BERR_INVALID_PARAMETER - Checker handle was invalid.
	BMRC_CHECKER_ERR_NO_CALLBACK_SET - No callback set.

See Also:
	BMRC_Checker_Disable
	BMRC_Checker_EnableCallback
	BMRC_Checker_DisableCallback
**************************************************************************/
BERR_Code BMRC_Checker_Enable
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
	);


/***************************************************************************
Summary:
	Disables a client's access checking.

Description:
	This function disables a client's access checking.

Returns:
	BERR_SUCCESS - Checker was successfully disabled.
	BERR_INVALID_PARAMETER - Checker handle was invalid.

See Also:
	BMRC_Checker_Enable
	BMRC_Checker_EnableCallback
	BMRC_Checker_DisableCallback
	BMRC_Checker_EnableCallback_isr
	BMRC_Checker_DisableCallback_isr
**************************************************************************/
BERR_Code BMRC_Checker_Disable
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
	);


/***************************************************************************
Summary:
	Enables a client's callback.

Description:
	This function enables a client's callback to be run when violations
	occur.  A checker must also be enabled for callbacks to work.

Returns:
	BERR_SUCCESS - Checker callback was successfully enabled.
	BERR_INVALID_PARAMETER - Checker handle was invalid.
	BMRC_CHECKER_ERR_NO_CALLBACK_SET - No callback set.

See Also:
	BMRC_Checker_Enable
	BMRC_Checker_Disable
	BMRC_Checker_DisableCallback
	BMRC_Checker_EnableCallback_isr
	BMRC_Checker_DisableCallback_isr
**************************************************************************/
BERR_Code BMRC_Checker_EnableCallback
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
	);


/***************************************************************************
Summary:
	Disables a client's callback.

Description:
	This function disables a client's callback, and the callback will
	not be run even when violations occur.

Returns:
	BERR_SUCCESS - Checker callback was successfully disabled.
	BERR_INVALID_PARAMETER - Checker handle was invalid.

See Also:
	BMRC_Checker_Enable
	BMRC_Checker_Disable
	BMRC_Checker_EnableCallback
	BMRC_Checker_EnableCallback_isr
	BMRC_Checker_DisableCallback_isr
**************************************************************************/
BERR_Code BMRC_Checker_DisableCallback
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
	);


/***************************************************************************
Summary:
	Enables a client's callback during an isr.

Description:
	This function enables a client's callback to be run when violations
	occur.  A checker must also be enabled for callbacks to work.

Returns:
	BERR_SUCCESS - Checker was successfully enabled.
	BERR_INVALID_PARAMETER - Checker handle was invalid.
	BMRC_CHECKER_ERR_NO_CALLBACK_SET - No callback set.

See Also:
	BMRC_Checker_DisableCallback_isr
	BMRC_Checker_Enable
	BMRC_Checker_Disable
	BMRC_Checker_EnableCallback
	BMRC_Checker_DisableCallback
**************************************************************************/
BERR_Code BMRC_Checker_EnableCallback_isr
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
	);


/***************************************************************************
Summary:
	Disables a client's callback during an isr.

Description:
	This function disables a client's callback, and the callback will
	not be run even when violations occur.

Returns:
	BERR_SUCCESS - Checker was successfully disabled.
	BERR_INVALID_PARAMETER - Checker handle was invalid.

See Also:
	BMRC_Checker_EnableCallback_isr
	BMRC_Checker_Enable
	BMRC_Checker_Disable
	BMRC_Checker_EnableCallback
	BMRC_Checker_DisableCallback
**************************************************************************/
BERR_Code BMRC_Checker_DisableCallback_isr
	( BMRC_Checker_Handle hChecker  /* [in] Checker handle */
	);


/***************************************************************************
Summary:
	Registers a callback to the checker.

Description:
	This function registers the callback that will be called when a
	violation occurs.

Returns:
	BERR_SUCCESS - Callback was successfully registered.

See Also:
**************************************************************************/
BERR_Code BMRC_Checker_SetCallback
	( BMRC_Checker_Handle hChecker,          /* [in] Checker handle */
	  const BMRC_CallbackFunc_isr pfCbFunc,  /* [in] Pointer to the callback function */
	  void *pvCbData1,                       /* [in] User defined callback data structure. */
	  int iCbData2                           /* [in] User defined callback value */
	);


/**************************************************************************
Summary:
	Gets a client's info.

Description:
	This function returns the client's information, such as client id and
	name in a client info structure.

Returns:

See Also:
**************************************************************************/
BERR_Code BMRC_Checker_GetClientInfo
	( BMRC_Handle hMrc,               /* [in] MRC Module Handle */
	  BMRC_Client eClient,            /* [in]  The client to get the info of */
	  BMRC_ClientInfo *pClientInfo    /* [out] Pointer to the client info structure */
	);

/**************************************************************************
Summary:
    Gets a client's name.

Description:
    This function return name of memory controller client.

Returns:

See Also:
**************************************************************************/
const char *BMRC_Checker_GetClientName (
    unsigned memc,      /* [in] memory controller number */
    unsigned clientId   /* [in] The client ID to get the name of */
  );


/**************************************************************************
Summary:
	Suspend instance of BMRC_Checker

Description:
	This function prepares instance of BMRC to power off hardware.
    It would save its internal state to memory, and prevent any further activity.
    Sate could be restored by calling BMRC_Resume.

See Also:
    BMRC_Checker_Resume
**************************************************************************/
void BMRC_Standby(
        BMRC_Handle hMrc /* [in] MRC Module Handle */
        );

/**************************************************************************
Summary:
	Resumes instance of BMRC_Checker

Description:
	This function restores instance of BMRC after powering up hardware.
    It would restore its internal state from memory saved by BMRC_Standby, and allow activity.

See Also:
    BMRC_Standby
**************************************************************************/
void BMRC_Resume(
        BMRC_Handle hMrc /* [in] MRC Module Handle */
        );

/**************************************************************************
Summary:
Print a warning to the console for all ARC's that are blocking reads or writes.
If user doesn't open MRC, blocking ARC's may cause silent loss of function.
**************************************************************************/
void BMRC_PrintBlockingArcs(BREG_Handle reg);

int BMRC_P_GetClientId (
    BMRC_Client eClient,   /* [in]  The client to get the info of */
    unsigned memc         /* [in] memory controller number */
  );

#ifdef __cplusplus
}
#endif

#endif
/* End of File */
