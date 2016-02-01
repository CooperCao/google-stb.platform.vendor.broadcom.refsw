/******************************************************************************
 *	  (c)2010-2013 Broadcom Corporation
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
 * 1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_Log: $
 *
 *****************************************************************************/


#ifndef __TR69C_DEFS_H__
#define __TR69C_DEFS_H__

/*
* The CPE parameters are represented by a tree where each node in the
 * tree is an array of TRxObjNode structures. Each array item represents
 * either a paramater item or an object containing a pointer to the next
 * level of TRxObjNode items that make up  parameters that make up the object.
 *
 * Each item that describes an object contains a pointer component, *objDetail,
 * that points to the array of parameter and object items that form the
 * object.
 * The function pointer, getTRxParam, may be NULL or point to
 * a hardware dependent function that returns the value of the parameter in
 * a string variable.
 * The function pointer, setTRxParam, is used to call a hardware dependent
 * function to set the parameter. If the function pointer is NULL the
 * parameter is not writeable. If the item is an object and the pointer is
 * NULL the rpc function AddObject/DeleteObject are not supported by that
 * item.
 *
 * The global variable thisTRxObjNode points to the current TRxObjNode entry
 * when the set/getTRxParam functions are called.
 *
 * If the node contains a single TRxObjNode item with TRxType of tInstance
 * then this item represents all instances of the object. This function is
 * responsible for keeping track of all instances associated with this
 * object. The parameter handling framework expects the following functionality
 * of the getTRxParam function which will need to maintain state information
 * as the framework accesses the instances. The framework will always call
 * the getTRxParam to access an instance on its way to access its
 * sub-objects/parameters. As the framework is traversing the parameter
 * tree it will call the getTRxParam function with one of the following
 * forms to validate and set the instance state before proceeding
 * to the next object/parameter level.
 *
 * To determine the existance of a specific instance:
 *  The getTRxParam function is called as follows:
 *
 *  node->getTRXParam(char **ParamInstance)
 *  If *ParamInstance is not NULL then it points to the value to be found.
 *  Returns: TRx_OK if ParameterInstance found. The function should set
 *                  a global state variable for use by the next level
 *                  get/setTRxParam functions to the ParameterInstance.
 *           TRx_ERR if ParameterInstance not found
 *
 *  To retrieve each of the instances in order:
 *  If *ParamInstance is NULL then it returns the first instance of the
 *  object.
 *  Returns: TRx_OK if any Instances exist. The *ParamInstance pointer points to the
 *                  name (instance number string) of the first instance.
 *                  The global instance state variable is set to the
 *                  instance returned in the value string.
 *           TRx_ERR no instances of  this object exist.
 *
 *  If *ParamInstance is (void *)(-1) then find the next instance relative
 * to the last instance returned.
 *  Returns: TRx_OK The *ParamInstance pointer points to the next instance.
 *                  instance name. Repeated calls with the returned
 *                  instance name from the previous call as the InstanceValue
 *                  will return all instances. The global instance state
 *                  variable is the instance returned in the value string.
 *          TRx_ERR no more instances.
 * See xxx for an example of how this is coded.
 */

#include <time.h>
#include "bstd_defs.h"

typedef enum {
    TRX_OK=0,
    TRX_REBOOT,
	TRX_REQUEST_DENIED			= 9001,  /**< Request denied (no reason specified). */
	TRX_ERR					   	= 9002,	/**< Internal error. */
	TRX_INVALID_ARGUMENTS    	= 9003,	/**< Invalid arguments. */
	TRX_RESOURCE_EXCEEDED    	= 9004,	/**< Resource exceeded.
										*  (when used in association with
										*  setParameterValues, this MUST not be
										*  used to indicate parameters in error)
										*/
	TRX_INVALID_PARAM_NAME   	= 9005,	/**< Invalid parameter name.
										*  (associated with set/getParameterValues,
										*  getParameterNames,set/getParameterAtrributes)
										*/
	TRX_INVALID_PARAM_TYPE   	= 9006,	/**< Invalid parameter type.
										*  (associated with set/getParameterValues)
										*/
	TRX_INVALID_PARAM_VALUE  	= 9007,	/**< Invalid parameter value.
										*  (associated with set/getParameterValues)
										*/
	TRX_SET_NON_WRITABLE_PARAM 	= 9008	/**< Attempt to set a non-writable parameter.
										*  (associated with setParameterValues)
										*/
}TRX_STATUS;

typedef enum {
    NOTIFICATION_OFF = 0,
    PASSIVE_NOTIFICATION,
    ACTIVE_NOTIFICATION
} eNotification;

typedef enum {
   eACSNeverContacted=0,
   eACSContacted,
   eACSInformed,
   eACSUpload,
   eACSDownloadReboot,
   eACSSetValueReboot,
   eACSAddObjectReboot,
   eACSDelObjectReboot,
   eACSRPCReboot
} eInformState;



extern eInformState  informState;

/* TR-069 session enum
 *  eSessionStart - sending 1st Inform
 *  eSessionAuthentication - sending 2nd Inform
 *  eSessionDeliveryConfirm - receiving InformResponse
 *  eSessionEnd - receiving 204 No Content or 200 OK
 */
typedef enum {
   eSessionUnknown,
   eSessionStart,
   eSessionAuthenticating,
   eSessionDeliveryConfirm,
   eSessionEnd
} eSessionState;


typedef enum {
    tUnknown=0,
    tObject,
    tString,
    tInt,
    tUnsigned,
    tBool,
    tDateTime,
    tBase64,
    tInstance,
    tStringSOnly,		/* Set strings only */
	tTrxTypeMax=255
} eTRxType;

typedef struct InstanceDesc {
    struct      InstanceDesc *next;
    struct      InstanceDesc *parent;
    int         instanceID;
    void        *hwUserData;
} InstanceDesc;

/* */
typedef struct InstanceDope {
    struct InstanceDope *next;
    InstanceDesc *instance;  /* set to currentInstance Desc*/
    char *pdata;
    unsigned notification:2;
    unsigned accessListIndex:1;
} InstanceDope;



typedef union TRxPAttrib {
    struct Attrib {
/*    eTRxType    etype:8;*/
    eTRxType    etype;
    unsigned    slength:16;
    unsigned    inhibitActiveNotify:1; /* set to always inhibit change notification: use on counters */
#if 0
    //unsigned    notification:2;
    //unsigned    accessListIndex:1;  /* is 0 for no Subscriber access or 1 to enable */
                                    /* subscriber access */
#endif
    } attrib;
    InstanceDesc    *instance;
} TRxPAttrib;

typedef TRX_STATUS (*TRxSETFUNC)(const char *value);
typedef TRX_STATUS (*TRxGETFUNC)(char **value);
#define TRXGFUNC(XX) TRX_STATUS XX (char **)
#define TRXSFUNC(XX) TRX_STATUS XX (const char *)

typedef struct TRxObjNode {
    const char  *name;
    TRxPAttrib  paramAttrib;
    TRxSETFUNC  setTRxParam;    /* only set if parameter is writeable */
    TRxGETFUNC  getTRxParam;
    void        *objDetail;
    InstanceDope *instanceDope;
#ifdef XML_DOC_SUPPORT
    unsigned char majorRev;
    unsigned char minorRev;
    unsigned int minEntries;
    unsigned int maxEntries;
    const char  *description;
    bool        supported;
#endif
} TRxObjNode;


const char *getValTypeStr(eTRxType);


/*
* Define CPEVARNAMEINSTANCE in standardparams.c to create an
 * instance of all the CPE parameter strings.
 * undef VARINSTANCE to create a extern to the string pointer
 * If CPEVARNAMEINSTANCE is defined
 * SVAR(X) creates a char string constant of X and labels it with
 * the variable name X.
 * SSVAR(X,Y) creates a char string constant of Y and labels it with
 * the variable name X. This is used for strings that can't be C labels.
 *
 * If CPEVARNAMEINSTANCE is NOT defined SVAR generates
 * a extern of the form   extern const char X[];
*/
#ifdef CPEVARNAMEINSTANCE
/*#define mkstr(S) # S  */
#define SVAR(X) const char X[]=#X
#define SSVAR(X,Y) const char X[]=#Y
#else
#define SVAR(X) extern char X[]
#define SSVAR(X,Y) extern char X[]
#endif

/* extern variables and functions that are already defined in TR69 framework */
extern const char instanceIDMASK[];
extern int getInstanceCountNoPathCheck( TRxObjNode *n);
extern TRxObjNode  *getCurrentNode(void);
extern InstanceDesc *getCurrentInstanceDesc(void);
extern InstanceDesc *getNewInstanceDesc( TRxObjNode *n, InstanceDesc *parent, int id);
extern InstanceDesc *findInstanceDesc( TRxObjNode *n, int id);
extern InstanceDesc *findInstanceDescNoPathCheck( TRxObjNode *n, int id);
extern int deleteInstanceDesc( TRxObjNode *n, int id);

extern void pushNode(TRxObjNode *n);
extern void popNode(void);
extern void pushInstance(InstanceDesc *idp);
extern void popInstance(void);


/* must match above enumberation of methods*/
typedef struct RpcMethods {
    unsigned   rpcGetRPCMethods:1;
    unsigned   rpcSetParameterValues:1;
    unsigned   rpcGetParameterValues:1;
    unsigned   rpcGetParameterNames:1;
    unsigned   rpcGetParameterAttributes:1;
    unsigned   rpcSetParameterAttributes:1;
    unsigned   rpcReboot:1;
    unsigned   rpcDownload:1;
    unsigned   rpcScheduleInform:1;
    unsigned   rpcFactoryReset:1;
} RpcMethods;

typedef struct vendorSpecificOption
{
	char *acsUrl;
	char *provisioningCode;
	unsigned int cwmpRetryMinimumWaitInterval;
	unsigned int cwmpRetryIntervalMultiplier;
} vendorSpecificOption;

#define DEF_CONFIG_LEN	128
typedef struct ACSState {
    char        *acsURL;        /* URL of ACS */
    char        *acsNewURL;     /* New URL if URL has been changed*/
    char        *acsUser;
    char        *acsPwd;
    time_t      informTime;     /* next ACS inform Time */
    time_t      informInterval; /* inform interval */
    int         informEnable;   /* True if inform to be performed*/
    int         maxEnvelopes;   /* Number of max env returned in inform response*/
    int         holdRequests;   /* hold request to ACS if true */
    int         noMoreRequests; /* don't send any more Req to ACS */
    RpcMethods  acsRpcMethods;  /* methods from GetRPCMethods response*/
    char        *parameterKey;  /* update key for ACS - may be NULL */
    char        *newParameterKey;  /* the pending key */
    char        *rebootCommandKey; /* key for reboot command key */
    char        *downloadCommandKey;    /* key for download cmd*/
    char        *connReqURL;
    char        *connReqPath;   /* path part of connReqURL -- used by listener */
    char        *connReqUser;
    char        *connReqPwd;
    char        *kickURL;
    char        upgradesManaged;
    char        *provisioningCode;
    int         retryCount;     /* reset on each ACS response*/
    int         fault;          /* last operation fault code */
    int         dlFaultStatus;  /* download fault status */
    char        *dlFaultMsg;    /* download fault message */
    time_t      startDLTime;    /* start download time */
    time_t      endDLTime;      /* complete download time*/
    int         enableCWMP;     /* turns CWMP on/off */
    int         defActiveNotificationThrottle; /* seconds to wait between notifications */
    int         cwmpRetryMinimumWaitInterval;
    int         cwmpRetryIntervalMultiplier;
    char        *dmAlias;       /* DeviceInfo.SupportedDataModel.Alias */
} ACSState;

typedef struct LimitNotificationInfo {
   char *parameterFullPathName; /* for example, IGD.ManagementServer.ManageableDeviceNumberOfEntries */
   int notificationPending; /* 1 = pending; 0 = nothing to send */
   int limitValue;          /* how often do we send active notification, in ms rather than Second */
   void*  func;             /* EventHandler, notification timer function */
   time_t /*CmsTimestamp*/ lastSent;   /* the time stamp at which the last notification was sent */
   struct LimitNotificationInfo *next;
} LimitNotificationInfo;

typedef struct LimitNotificationQInfo {
   int count;
   LimitNotificationInfo *limitEntry;
} LimitNotificationQInfo;

/* These definitions correspond to informEventStr array which
 * we use to map these integers value (saved in informEventList)
 * to string returned in inform message
*/
#define INFORM_EVENT_BOOTSTRAP                0
#define INFORM_EVENT_BOOT                     1
#define INFORM_EVENT_PERIODIC                 2
#define INFORM_EVENT_SCHEDULED                3
#define INFORM_EVENT_VALUE_CHANGE             4
#define INFORM_EVENT_KICKED                   5
#define INFORM_EVENT_CONNECTION_REQUEST       6
#define INFORM_EVENT_TRANSER_COMPLETE         7
#define INFORM_EVENT_DIAGNOSTICS_COMPLETE     8
#define INFORM_EVENT_REBOOT_METHOD            9
#define INFORM_EVENT_SCHEDULE_METHOD          10
#define INFORM_EVENT_DOWNLOAD_METHOD          11
#define INFORM_EVENT_UPLOAD_METHOD            12

/*!\enum CmsRet
 * \brief Return codes for all external functions, and some internal functions too.
 *
 * Codes from 9000-9799 are reserved for TR69C return values.
 * All Broadcom return codes should start at 9800.
 */
typedef enum
{
   CMSRET_SUCCESS              = 0,     /**<Success. */
   CMSRET_METHOD_NOT_SUPPORTED = 9000,  /**<Method not supported. */
   CMSRET_REQUEST_DENIED       = 9001,  /**< Request denied (no reason specified). */
   CMSRET_INTERNAL_ERROR       = 9002,  /**< Internal error. */
   CMSRET_INVALID_ARGUMENTS    = 9003,  /**< Invalid arguments. */
   CMSRET_RESOURCE_EXCEEDED    = 9004,  /**< Resource exceeded.
                                        *  (when used in association with
                                        *  setParameterValues, this MUST not be
                                        *  used to indicate parameters in error)
                                        */
   CMSRET_INVALID_PARAM_NAME   = 9005,  /**< Invalid parameter name.
                                        *  (associated with set/getParameterValues,
                                        *  getParameterNames,set/getParameterAtrributes)
                                        */
   CMSRET_INVALID_PARAM_TYPE   = 9006,  /**< Invalid parameter type.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_INVALID_PARAM_VALUE  = 9007,  /**< Invalid parameter value.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_SET_NON_WRITABLE_PARAM = 9008,/**< Attempt to set a non-writable parameter.
                                        *  (associated with setParameterValues)
                                        */
   CMSRET_NOTIFICATION_REQ_REJECTED = 9009, /**< Notification request rejected.
                                            *  (associated with setParameterAttributes)
                                            */
   CMSRET_DOWNLOAD_FAILURE     = 9010,  /**< Download failure.
                                         *  (associated with download or transferComplete)
                                         */
   CMSRET_UPLOAD_FAILURE       = 9011,  /**< Upload failure.
                                        *  (associated with upload or transferComplete)
                                        */
   CMSRET_FILE_TRANSFER_AUTH_FAILURE = 9012,  /**< File transfer server authentication
                                              *  failure.
                                              *  (associated with upload, download
                                              *  or transferComplete)
                                              */
   CMSRET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,/**< Unsupported protocol for file
                                                    *  transfer.
                                                    *  (associated with upload or
                                                    *  download)
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_JOIN_MULTICAST = 9014,/**< File transfer failure,
                                                    *  unable to join multicast
                                                    *  group.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER = 9015,/**< File transfer failure,
                                                    *  unable to contact file server.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_ACCESS_FILE = 9016,/**< File transfer failure,
                                                    *  unable to access file.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_COMPLETE = 9017,/**< File transfer failure,
                                                    *  unable to complete download.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CORRUPTED = 9018,/**< File transfer failure,
                                                    *  file corrupted.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR = 9019,/**< File transfer failure,
                                                    *  file authentication error.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_TIMEOUT = 9020,/**< File transfer failure,
                                                    *  download timeout.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CANCELLATION_NOT_ALLOW = 9021,/**< File transfer failure,
                                                    *  cancellation not permitted.
                                                    */
   CMSRET_INVALID_UUID_FORMAT = 9022,/**< Invalid UUID Format
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_UNKNOWN_EE = 9023,/**< Unknown Execution Environment
                                                    * (associated with ChangeDUState)
                                                    */

   CMSRET_EE_DISABLED = 9024,/**< Execution Environment disabled
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_DU_EE_MISMATCH = 9025,/**< Execution Environment and Deployment Unit mismatch
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_DUPLICATE = 9026,/**< Duplicate Deployment Unit
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED = 9027,/**< System resources exceeded
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_UNKNOWN = 9028,/**< Unknown Deployment Unit
                                                    * (associated with ChangeDUState:update/uninstall)
                                                    */
   CMSRET_DU_STATE_INVALID = 9029,/**< Invalid Deployment Unit State
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED = 9030,/**< Invalid Deployment Unit Update, downgrade not permitted
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_VERSION_NOT_SPECIFIED = 9031,/**< Invalid Deployment Unit Update, version not specified
                                                    * (associated with ChangeDUState:update)
                                                    */

   CMSRET_DU_UPDATE_VERSION_EXISTED= 9032,/**< Invalid Deployment Unit Update, version already exists
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_SUCCESS_REBOOT_REQUIRED = 9800, /**< Config successful, but requires reboot to take effect. */
   CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED = 9801,  /**<Success, but some unrecognized data was ignored. */
   CMSRET_SUCCESS_OBJECT_UNCHANGED = 9802,  /**<Success, furthermore object has not changed, returned by STL handler functions. */
   CMSRET_FAIL_REBOOT_REQUIRED = 9803,  /**<Config failed, and now system is in a bad state requiring reboot. */
   CMSRET_NO_MORE_INSTANCES = 9804,     /**<getnext operation cannot find any more instances to return. */
   CMSRET_MDM_TREE_ERROR = 9805,         /**<Error during MDM tree traversal */
   CMSRET_WOULD_DEADLOCK = 9806, /**< Caller is requesting a lock while holding the same lock or a different one. */
   CMSRET_LOCK_REQUIRED = 9807,  /**< The MDM lock is required for this operation. */
   CMSRET_OP_INTR = 9808,      /**<Operation was interrupted, most likely by a Linux signal. */
   CMSRET_TIMED_OUT = 9809,     /**<Operation timed out. */
   CMSRET_DISCONNECTED = 9810,  /**< Communications link is disconnected. */
   CMSRET_MSG_BOUNCED = 9811,   /**< Msg was sent to a process not running, and the
                                 *   bounceIfNotRunning flag was set on the header.  */
   CMSRET_OP_ABORTED_BY_USER = 9812,  /**< Operation was aborted/discontinued by the user */
   CMSRET_RECURSION_ERROR = 9817,     /**< too many levels of recursion */
   CMSRET_OPEN_FILE_ERROR = 9818,     /**< open file error */
   CMSRET_KEY_GENERATION_ERROR = 9830,     /** certificate key generation error */
   CMSRET_INVALID_CERT_REQ = 9831,     /** requested certificate does not match with issued certificate */
   CMSRET_INVALID_CERT_SUBJECT = 9832,     /** certificate has invalid subject information */
   CMSRET_OBJECT_NOT_FOUND = 9840,     /** failed to find object */

   CMSRET_INVALID_FILENAME = 9850,  /**< filename was not given for download */
   CMSRET_INVALID_IMAGE = 9851,     /**< bad image was given for download */
   CMSRET_INVALID_CONFIG_FILE = 9852,  /**< invalid config file was detected */
   CMSRET_CONFIG_PSI = 9853,         /**< old PSI/3.x config file was detected */
   CMSRET_IMAGE_FLASH_FAILED = 9854, /**< could not write the image to flash */
   CMSRET_RESOURCE_NOT_CONFIGURED = 9855 /**< requested resource is not configured/found */

} CmsRet;

#endif   /* TR69C_DEFS_H */
