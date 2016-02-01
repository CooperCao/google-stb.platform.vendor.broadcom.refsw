/***************************************************************************
 *     (c)2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/

/** @file nexus_scm.h
    This is a NEXUS interface to SCM binary running on SAGE.
    The SCM interface is generic and does not have any dependencies on a
    specific SCM.
    The NEXUS SCM interface provides following functionalities to the
    caller:
    - SCM loading and authentication.
    - SCM shutdown.
    - SCM message passing interface.

    SCM loading and authentication happens when the caller opens SCM
    channel and obtains a channel handle. The SCM will be
    authenticated as a part of loading process and communication
    channel will be establised. If this process fails at any point,
    the interface will return an error.

    SCM shutdown happens when the caller closes SCM channel
    handle. The interface will cleanup the data structures and will
    prepare SAGE for loading next SCM.

    SCM message passing interface is responsible for relaying messages
    to SCM running on SAGE secure processor and for returning
    responses from SAGE back to the caller.
 */

#ifndef NEXUS_SCM_H__
#define NEXUS_SCM_H__

#include "nexus_types.h"
#include "nexus_memory.h"

#include "nexus_scm_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
    @brief NEXUS Scm handle.

    Handle for the SCM main module. This handle is returned by the
    open call and should be subsequently used for any operations on
    the SCM module.
 */
typedef struct NEXUS_Scm *NEXUS_ScmHandle;

/**
    @brief NEXUS Scm Channel handle.

    Handle for the SCM Channel. User must open SCM Channel after
    obtaining handle for the SCM main module. Channel handle must be
    used for communicating with the SCM instance.
 */
typedef struct NEXUS_ScmChannel *NEXUS_ScmChannelHandle;

/** @struct NEXUS_ScmOpenSettings

    Settings used for opening SCM main module
  */
/** @var NEXUS_ScmOpenSettings::watchdogCallback

    Deprecated parameter, will be removed in the future.
 */
typedef struct NEXUS_ScmOpenSettings
{
    NEXUS_CallbackDesc watchdogCallback;
} NEXUS_ScmOpenSettings;

/**
   @brief Call to fill provided structure with default settings.

   These settings can be modified at a later time before calling the
   open call.

   @param pSettings a pointer to the structure to fill with
   settings.
   @return Nothing
 */
void NEXUS_Scm_GetDefaultOpenSettings(
    NEXUS_ScmOpenSettings *pSettings /* [out] */
    );
/**
   @brief Call to open main SCM module and to obtain SCM module handle.

   The handle is used to open SCM channel to communicate with a particular
   SCM. This function does not involve any interactions with SAGE secure
   processor but used to allocate resources.

   @param pSettings A pointer to the structure containing setting for the
   module.
   @return NEXUS_ScmHandle A handle for the module in case of sussess, NULL
   in case of failure.
 */
NEXUS_ScmHandle NEXUS_Scm_Open( /* attr{destructor=NEXUS_Scm_Close} */
    const NEXUS_ScmOpenSettings *pSettings /* attr{null_allowed=y} */
    );

/**
   @brief Call to close main SCM module.

   This function should be called to free resources allocated by the SCM module.
   After this call the handle is no longer valid and should not be used.

   @param scm A handle for the scm module to be closed.
   @return Nothing
 */
void NEXUS_Scm_Close(
    NEXUS_ScmHandle scm
    );

/*
 */

/** @enum NEXUS_ScmType

    @brief SCM Type definitions.

    Defines all possible SCM types that can be loaded.

    @var NEXUS_ScmType_Undefined
    Cosntant indicating invalid SCM type. Used for initialization of variable.

    @var NEXUS_ScmType_Generic
    Generic SCM.

    @var NEXUS_ScmType_Arris
    SCM-A (ARRIS)

    @var NEXUS_ScmType_Cisco
    SCM-C (Cisco)

    @var NEXUS_ScmType_Shutdown
    Shudown SCM type. Used to place SAGE in reset.
*/
typedef enum NEXUS_ScmType
{
    NEXUS_ScmType_Undefined = 0,
    NEXUS_ScmType_Generic,
    NEXUS_ScmType_Arris,
    NEXUS_ScmType_Cisco,
    NEXUS_ScmType_Shutdown
} NEXUS_ScmType;

/** @struct NEXUS_ScmChannelSettings

    This structure contains settings for opening SCM channel.

    @see NEXUS_ScmChannel_GetDefaultSettings, NEXUS_Scm_CreateChannel
 */
/** @var NEXUS_ScmChannelSettings::type

    Type of SCM to load.
 */
/** @var NEXUS_ScmChannelSettings::successCallback

    Deprecated parameter, will be removed in the future
 */
/** @var NEXUS_ScmChannelSettings::errorCallback

    Deprecated parameter, will be removed in the future
 */
/** @var NEXUS_ScmChannelSettings::heap

    Deprecated parameter, will be removed in the future
 */

typedef struct NEXUS_ScmChannelSettings
{
    NEXUS_ScmType type;
    NEXUS_CallbackDesc successCallback;
    NEXUS_CallbackDesc errorCallback;
    NEXUS_HeapHandle heap;
} NEXUS_ScmChannelSettings;

/**
   @brief Get default settings for opening SCM channel.

   This function accepts pointer to the settings structure and populates
   passed structure with default values, effectively initializing the stucture.

   @param pSettings A pointer to the structure to be filled with default
   parameters.

   @return None.

   @see NEXUS_Scm_CreateChannel
 */
void NEXUS_ScmChannel_GetDefaultSettings(
    NEXUS_ScmChannelSettings *pSettings /* [out] */
    );

/**
   Create SCM channel.

   This function will create SCM channel and return channel handle that can be
   used for communication with the SCM. The loading process consist of
   locating SCM image corresponding to the SCM type provided in the settings,
   authenticating the SCM image and establishing communications with the SCM.
   If any of these steps fail the function will return value NULL. If all
   steps are successful, the function will return SCM channel handle.

   @param scm A NEXUS SCM module handle.
   @param pSettings A pointer to the filled settings structure. Desired SCM
   type should be placed in this structure.

   @return SCM channel handle if loading of the SCM was successful, NULL
   otherwise.

   @see NEXUS_ScmChannel_GetDefaultSettings, NEXUS_Scm_DestroyChannel
 */
NEXUS_ScmChannelHandle NEXUS_Scm_CreateChannel(
    NEXUS_ScmHandle scm,
    const NEXUS_ScmChannelSettings *pSettings
    );

/**
   Destroy SCM channel.

   This function will shutdown SCM and prepare SAGE for loading of the
   new SCM binary. It must be called when SCM is no longer needed and
   must be terminated. The SCM must be ready for shutdown else the
   shutdown will not be successful and next attempt to load the SCM
   will fail. Command to bring SCM in to a ready to shudown state is
   SCM specific and outside of the scope of this document.

   @param channel A NEXUS SCM channel handle that was obtained by calling
   NEXUS_Scm_CreateChannel() call.

   @return None.
 */
void NEXUS_Scm_DestroyChannel(
    NEXUS_ScmChannelHandle channel
    );

/**
   @def SCM_CMD_SIZE

   Size of the SCM command buffer, also maximum size of the SCM command.

   @def SCM_RSP_SIZE

   Size of the SCM response buffer, also maximum size of the SCM response.
 */
#define SCM_CMD_SIZE 0x300
#define SCM_RSP_SIZE 0x800

/** @struct NEXUS_ScmCommand
    This structure is used to pass commands to the SCM and retrieve responses
    from the SCM.
 */
/** @var struct NEXUS_ScmCommand::scmCommandId
    A variable containing current command id. The value of command id is SCM
    specific and outside of the scope of this document. Command id can be any
    32 bit number.
 */
/** @var struct NEXUS_ScmCommand::cmd
    A pointer to the buffer containing command body.
 */
/** @var struct NEXUS_ScmCommand::cmd_size
    A varible containing size of the command body. The maximum size is defined
    by the SCM_CMD_SIDE constant. The actual command size is command specific.
 */
/** @var struct NEXUS_ScmCommand::rsp
    A pointer to a buffer in which SCM response should be placed. The SCM
    response will be copied from the SCM controlled memory in to an application
    buffer pointed by this pointer.
 */
/** @var struct NEXUS_ScmCommand::rsp_size
    A variable containing size of the response buffer. The maxumum possilbe
    size is defined by the SCM_RSP_SIZE constant. NEXUS SCM module will copy
    SCM response up to rsp_size bytes in to the provided buffer. If actual
    response size is larger than rsp_size the response data will be truncated.
    Response size for each SCM command is SCM specific and outside of the scope
    of this document.
 */

typedef struct NEXUS_ScmCommand {
    uint32_t scmCommandId;
    uint32_t * cmd;
    uint32_t cmd_size;
    uint32_t * rsp;
    uint32_t rsp_size;
} NEXUS_ScmCommand;

/**
   Send command to the SCM and wait for response

   This function will send command to the SCM and wait for response
   from the SCM for up to one second. Function will return when
   response is received or when timeout occurs. Please note that
   return value of this function only indicates success or failure of
   communicating with the SCM and not in any way indicative of the
   actual SCM response.

   @param channel A NEXUS SCM channel handle.
   @param pCommand A pointer to the NEXUS_ScmCommand structure containing
   command and response buffers.

   @return NEXUS_SUCCESS if command processed successfully or
   NEXUS_NOT_AVAILABLE if communication error occurred.

   @see NEXUS_Scm_CreateChannel, NEXUS_Scm_DestroyChannel
 */
NEXUS_Error NEXUS_ScmChannel_SendCommand(
    NEXUS_ScmChannelHandle channel,
    const NEXUS_ScmCommand *pCommand
    );

/** @struct NEXUS_ScmChannelStatus

   Structure contains status of the SCM.

   This structure is used for obtaining status of currently loaded SCM.
*/
/** @var struct NEXUS_ScmChannelStatus::type
   A variable containing type  of currently loaded SCM
*/
/** @var struct NEXUS_ScmChannelStatus::otpVersion
   A variable containing OTP field with version (epoch) of the SCM that is
   currently loaded. Only OTP field corresponding to currently loaded SCM is
   returned. Some SCMs do not have version stored in OTP. For such SCMs this
   field will contain 0s.
*/
/** @var struct NEXUS_ScmChannelStatus::imageVersion
   A variable containing version encoded in the SCM binary (image). The version
   is read from the SCM image during SCM loading process and maintained by
   the SCM module. The image version is maintained only for currently loaded
   SCM image and there is no way to retrieve version from an unloaded SCM.
 */
typedef struct NEXUS_ScmChannelStatus {
    NEXUS_ScmType type;
    uint32_t otpVersion[4];
    uint32_t imageVersion;
} NEXUS_ScmChannelStatus;

/**
   Get SCM channel status information

   This function will retrieve SCM channel status information by returning
   internal variables, maintained by the NEXUS SCM module.

   @param channel A valid NEXUS SCM channel handle
   @param pStatus A pointer to the status data structure to fill with the
   status information.

   @return NEXUS_SUCCESS in case of successful call, error code otherwise. If
   value other than NEXUS_SUCCESS is returned by this call, the content of
   structure shoud be considered invalid and should not be used.
 */
NEXUS_Error NEXUS_ScmGetChannelStatus(
    NEXUS_ScmChannelHandle channel,
    NEXUS_ScmChannelStatus *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SCM_H__ */
