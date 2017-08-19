/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#ifndef BIP_STATUS_H
#define BIP_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BIP_MAKE_STATUS_CODE
#undef BIP_MAKE_STATUS_CODE
#endif
#define BIP_MAKE_STATUS_CODE(name,class,id)  name = NEXUS_MAKE_ERR_CODE(class,id),

typedef enum BIP_StatusCode
{  /* Start of the BIP_StatusCode enum definitions. */

    /* When we include "bip_statuscodes.h", the BIP_MAKE_STATUS_CODE macro will
     * build a list of statuscodes and their associated text that looks
     * similar to this:
     *
     *      BIP_ERR_NOT_INITIALIZED      = NEXUS_MAKE_ERR_CODE(0xB1F, 0x1),
     *      BIP_ERR_INVALID_PARAMETER    = NEXUS_MAKE_ERR_CODE(0xB1F, 0x2),
     *      BIP_ERR_OUT_OF_SYSTEM_MEMORY = NEXUS_MAKE_ERR_CODE(0xB1F, 0x3),
     *
     * And thus define each of the BIP_StatusCode enum values.  */

    #include "bip_statuscodes.h"

    /* Define the end of our reserved range. */
    BIP_STATUS_MAX = NEXUS_MAKE_ERR_CODE(BIP_NEXUS_STATUS_CODE_ID, 0xFFFF)

} BIP_StatusCode;   /* End of the BIP_StatusCode enum definitions. */

/**
Summary:
Convert a BIP_StatusCode to a text string.

Description:
This function searches a table of BIP_StatusCodes for the specified
statusCode and returns a pointer to the associated text string.
The lookup table is built at compile time and will use statusCode
definitions below.
**/
const char *  BIP_StatusGetText( BIP_StatusCode statusCode);

/**
 * Summary:
 * A pair of macros to provide printf-style formatting for BIP_Status codes.
 *
 * Description:
 *
 **/
#define BIP_STATUS_ARG(bs)    (bs), BIP_StatusGetText(bs)
#define BIP_STATUS_FMT       "0x%x: %s"


/**
Summary:
Encode an "errno" value into a BIP_Status value

Description:
This function takes an "errno" integer and encodes it into a BIP_Status
value.  If the specified errno value is invalid (e.g., negative), then
BIP_ERR_OS_ERRNO_INVALID is returned.
**/
BIP_Status  BIP_StatusFromErrno(int errnoValue);

/**
Summary:
Extract the errno value from a BIP_Status.  The BIP_Status value must range from
BIP_ERR_OS_ERRNO_MIN to BIP_ERR_OS_ERRNO_MAX (inclusive).
**/
int  BIP_StatusToErrno(BIP_Status bipStatus);


/**
Summary:
Print an error message that include the source file and line number and describes
the specified BIP_Status value.  It is printed using BDBG_ERR() regardless of
whether the BIP_Status is success, informational, or error.
**/
BIP_Status BIP_Status_PrintError(const char *pFileName, int line, BIP_Status bipStatus);

/**
Summary:
Return the thread ID for the current process.
**/
int  BIP_Status_GetMyThreadId(void);


/**
Summary:
BIP Settings validation macros.

Description:
These macros work just like the BDBG_OBJECT macros, but they are for detecting when an
API is called with a settings structure that wasn't initialized with either GetDefaultSettings()
or a GetSettings() call.

Refer to the BERR_TRACE macro for details.
**/

struct bip_settings_obj
{
    const char *bip_settings_obj_id;
};
void BIP_Settings_Print(const void *ptr, size_t size, const struct bip_settings_obj *obj, const char *id, const char *file, unsigned line);


#if 0  /* ==================== GARYWASHERE - Start of Original Code ==================== */
#define BIP_SETTINGS_ASSERT(ptr,name)                                                         \
do {                                                                                          \
    void *pCheck = (ptr);       /* To avoid compiler warning. */                              \
    BSTD_UNUSED(pCheck);                                                                      \
    (                                                                                         \
        (                                                                                     \
            (pCheck) &&                                                                       \
            (ptr)->bip_settings_object_##name.bip_settings_obj_id==bip_settings_id__##name    \
        ) ?                                                                                   \
            (void) 0                                                                          \
        :                                                                                     \
            (   BIP_Settings_Print(ptr,                                                       \
                               sizeof(*ptr),                                                  \
                               &(ptr)->bip_settings_object_##name,                            \
                               bip_settings_id__##name,                                       \
                               __FILE__,                                                      \
                               __LINE__)                                                      \
                ,                                                                             \
                BDBG_ASSERT(0)                                                                \
            )                                                                                 \
    );                                                                                        \
} while(0);
#else  /* ==================== GARYWASHERE - Start of Modified Code ==================== */

/* Allow NULL pointer. But if pointer is non-NULL, it must point to an initialized struct. */
#define BIP_SETTINGS_ASSERT(ptr,name)                                                         \
do {                                                                                          \
    const void *pCheck = (ptr);       /* To avoid compiler warning. */                        \
    BSTD_UNUSED(pCheck);                                                                      \
    (                                                                                         \
        (                                                                                     \
            (pCheck) &&                                                                       \
            (ptr)->bip_settings_object_##name.bip_settings_obj_id!=bip_settings_id__##name    \
        ) ?                                                                                   \
            (   BIP_Settings_Print(ptr,                                                       \
                        sizeof(*ptr),                                                         \
                        &(ptr)->bip_settings_object_##name,                                   \
                        bip_settings_id__##name,                                              \
                        __FILE__,                                                             \
                        __LINE__)                                                             \
             ,                                                                                \
                BDBG_ASSERT(0)                                                                \
             )                                                                                \
        :                                                                                     \
            (void) 0                                                                          \
    );                                                                                        \
} while(0);
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

#define BIP_SETTINGS_ID(name) const char bip_settings_id__##name[]= "#" #name
#define BIP_SETTINGS_ID_DECLARE(name) extern const char bip_settings_id__##name[]
#define BIP_SETTINGS(name) struct bip_settings_obj bip_settings_object_##name;
#define BIP_SETTINGS_SET(ptr,name) (ptr)->bip_settings_object_##name.bip_settings_obj_id=bip_settings_id__##name



/**
Summary:
Macros for implementing GetDefault() macros.

Description:
For a Settings structure like this:

    typedef struct BIP_MediaInfoGenerateSettings
    {
        BIP_SETTINGS(BIP_MediaInfoGenerateSettings)

        BIP_MediaInfoType infoTypeBitmask;
        bool              reGenerate;
        int psiAcquireTimeoutInMs;
    } BIP_MediaInfoGenerateSettings;

Use this to declare the settings ID as external:

    BIP_SETTINGS_ID_DECLARE(BIP_MediaInfoGenerateSettings);

Then define the GetDefault() macro like this:

    #define BIP_MediaInfo_GetDefaultGenerateSettings(pSettings)                     \
            BIP_SETTINGS_INIT_GET_DEFAULT_BEGIN(pSettings, BIP_MediaInfoGenerateSettings) \
            (pSettings)->infoTypeBitmask   = BIP_MediaStreamInfoType_eAll;          \
            (pSettings)->reGenerate        = false;                                 \
            (pSettings)->psiAcquireTimeoutInMs = 2000;                              \
            (pSettings)->hls.segmentDurationInMs=1000;                              \
            (pSettings)->mpegDash.segmentDurationInMs =1000;                        \
            BIP_SETTINGS_INIT_GET_DEFAULT_END

 **/
#define BIP_SETTINGS_GET_DEFAULT_BEGIN(pSettings, settingsType)  \
    do {                                                         \
        settingsType *pCheck = (pSettings);                      \
        BDBG_ASSERT( pCheck);                                    \
        /* Set everything to zero. */                            \
        B_Os_Memset( (pSettings), 0, sizeof( settingsType ));    \
        BIP_SETTINGS_SET((pSettings), settingsType);

#define BIP_SETTINGS_GET_DEFAULT_END   \
    } while(0);


/**
Summary:
Low-level interface to BDBG_ debug logging.

Description:
This function provides a low-level interface to the BDBG_ logging system.
It can be modified to provide custom formatting of the messages that it logs.

Unfortunately, the BIP_Status_BdbgPrintmsg() relies on the use of the BDBG_P_PRINTMSG_PRIV
macro, which is considered to be private to bdbg.h.  So this is probably best used for debug
purposes.
**/
/* #define USE_BIP_BDBG_PRINTMSG_FOR_LOGGING 1 */ /* Leave disabled for now. */
#if USE_BIP_BDBG_PRINTMSG_FOR_LOGGING
void BIP_Status_BdbgPrintmsg(const char * func, int line, BDBG_pDebugModuleFile pModule, BDBG_Level lvl,  const char *fmt, ...);

#define BIP_MSG(fmt)  BIP_Status_BdbgPrintmsg(BSTD_FUNCTION, __LINE__, &b_dbg_module, BDBG_eMsg, fmt);
#define BIP_WRN(fmt)  BIP_Status_BdbgPrintmsg(BSTD_FUNCTION, __LINE__, &b_dbg_module, BDBG_eWrn, fmt);
#define BIP_ERR(fmt)  BIP_Status_BdbgPrintmsg(BSTD_FUNCTION, __LINE__, &b_dbg_module, BDBG_eErr, fmt);
#define BIP_LOG(fmt)  BIP_Status_BdbgPrintmsg(BSTD_FUNCTION, __LINE__, &b_dbg_module, BDBG_eLog, fmt);
#else
#define BIP_MSG(fmt)  BDBG_MSG(fmt)
#define BIP_WRN(fmt)  BDBG_WRN(fmt)
#define BIP_ERR(fmt)  BDBG_ERR(fmt)
#define BIP_LOG(fmt)  BDBG_LOG(fmt)
#endif


/**
Summary:
BIP equivalent of BERR_TRACE.

Description:
This should work like BERR_TRACE but will handle printing the descriptive
text for BIP-specific status codes.

Refer to the BERR_TRACE macro for details.
**/

#if BDBG_DEBUG_BUILD
#if B_REFSW_DEBUG_COMPACT_ERR
#define BIP_ERR_TRACE(code) BIP_ERR_TRACE_P(__FILE__, __LINE__, "", code)
#else
#define BIP_ERR_TRACE(code) BIP_ERR_TRACE_P(__FILE__, __LINE__, #code, code)
#endif
#else
#define BIP_ERR_TRACE(code)  (code)
#endif

/* BIP_ERR_TRACE_P is "private" and should only be used by thing in this file. */
#define BIP_ERR_TRACE_P(file, line, text, code)                                                           \
        (  (BIP_Status_PrintError(file, line, code) != BIP_SUCCESS)                                       \
            ?   (                                                                                         \
                    (BIP_Status)BERR_TRACE((code))                                                        \
                )                                                                                         \
                : ((BIP_Status)code)                                                                      \
        )

/**
Summary:
Here are prefixes that can be used to prefix BDBG_xxx messages with additional info
like line number and function name.

Usage is like this:
    BDBG_MSG((BIP_MSG_PRE_FMT "This is a test %d %d %d..." BIP_MSG_PRE_ARG, 1, 2, 3));

which will result in output like this (for BIP_MSG_PRE_STYLE == 1):

    00:00:00.287 url_test: 512: main():: This is a test: 1 2 3...
**/

/* #define BIP_MSG_PRE_STYLE  5 */
#define BIP_MSG_PRE_STYLE  2  /* Use style 2 because it's more efficient than 5. */

#if  BIP_MSG_PRE_STYLE == 1         /* <Line number>: <function name>():: */
    /*   00:00:00.287 url_test: 512: main():: This is a test: 1 2 3...  */
    #define BIP_MSG_PRE_FMT  "%d: %s():: "
    #define BIP_MSG_PRE_ARG ,__LINE__,BSTD_FUNCTION

#elif  BIP_MSG_PRE_STYLE == 2        /* <Line number>: [<thread id>] <function name>():: */
    /*   00:00:00.287 url_test: 512: [b60c3450] main():: This is a test: 1 2 3...  */
    #define BIP_MSG_PRE_FMT  "%d: [%x] %s():: "
    #define BIP_MSG_PRE_ARG ,__LINE__, BIP_Status_GetMyThreadId(),BSTD_FUNCTION

#elif  BIP_MSG_PRE_STYLE == 3        /* <Line number>: [<thread name>] <function name>():: */
    /*   00:00:00.287 url_test: 512: [BipTmr] main():: This is a test: 1 2 3...  */
    #define BIP_MSG_PRE_FMT  "%d: [%s] %s():: "
    #define BIP_MSG_PRE_ARG ,__LINE__, B_Thread_GetMyName(),BSTD_FUNCTION

#elif  BIP_MSG_PRE_STYLE == 4        /* <Line number>: [<thread id>:<thread name>] <function name>():: */
    /*   00:00:00.287 url_test: 512: [b60c3450:BipTmr] main():: This is a test: 1 2 3...  */
    #define BIP_MSG_PRE_FMT  "%4d: [%8x:%-12s] %s():: "
    #define BIP_MSG_PRE_ARG ,__LINE__, BIP_Status_GetMyThreadId(), B_Thread_GetMyName(),BSTD_FUNCTION

#elif  BIP_MSG_PRE_STYLE == 5        /* <Line number>: [<thread id>|<thread name>] <function name>():: */
    /*   00:00:00.287 url_test: 512: [b60c3450:BipTmr] main():: This is a test: 1 2 3...  */
    #define BIP_MSG_PRE_FMT  "%4d: [%*.*x%*.*s] %s():: "
    #define BIP_MSG_PRE_ARG ,__LINE__,                                \
        B_Thread_GetMyName()==NULL ? 12 : 0,                          \
        B_Thread_GetMyName()==NULL ?  8 : 0,                          \
        B_Thread_GetMyName()==NULL ? BIP_Status_GetMyThreadId() : 0,  \
        B_Thread_GetMyName()==NULL ? 0 : 12,                          \
        B_Thread_GetMyName()==NULL ? 0 : 12,                          \
        B_Thread_GetMyName()==NULL ? "" : B_Thread_GetMyName(),       \
      BSTD_FUNCTION

#else
    #error BIP_MSG_PRE_STYLE has not been defined!
#endif

/**
Summary:
A macro that can check for success, and do multiple things upon failure.

It evaluates an expression, and if true (success), then does nothing.
But if false, the following things are done.
  1. Evaluates and saves the "errorValue" arg in case it has a reference to "errno".
  2. Prints a caller-specified message using BDBG_ERR.
  3. Calls BIP_ERR_TRACE with a caller-specified errorValue.
  4. Sets a caller-specified errorVariable to the caller-specified errorValue.
  5. Performs a "goto" to a caller-specified label.

errorMessage can be a printf-style format string plus arguments, for example:

        hUrl =  BIP_Url_Create( thisUrl  );
        BIP_CHECK_GOTO( (hUrl), ("Unable to parse URL: \"%s\"",thisUrl), myErrorLabel1, BIP_ERR_INVALID_PARAMETER, rc);

Which would result in output like this:

  ### 00:00:00.292 url_test: Unable to parse URL: "%http://this_is_a_test.com"
  !!!Error BERR_INVALID_PARAMETER(0x2) at /build/gskerl/gitRepos/refsw/GarysBaseline/nexus/lib/bip/examples/url/url_test.c:514

The "errorValue" arg is evaluated and saved before doing anything else, so it is safe
for the errorValue arg to have a reference to "errno", like this:

        dirErr = stat( pMediaFileInfoInputSettings->pInfoFilesDirectoryPath, &infoFileStats );
        BIP_CHECK_GOTO(( dirErr>=0 ), ( "%s: Invalid Info Directory Path", BSTD_FUNCTION ), error, BIP_StatusFromErrno(errno), rc );
**/
#define BIP_CHECK_GOTO(                                                                     \
    expression,     /* Condition expected to be true for success.               */          \
    errorMessage,   /* If expression false, print this with BDBG_ERR.           */          \
    errorLabel,     /* If expression false, goto this label.                    */          \
    errorValue,     /* If expression false, set errorVariable to this value.    */          \
    errorVariable   /* If expression false, set this to errorValue              */          \
    )                                                                                       \
    do {                                                                                    \
        if (!(expression)) {                                                                \
            BIP_Status  myErrorValue = errorValue; /* in case errorValue refers to errno */ \
            BDBG_ERR(errorMessage);                                                         \
            errorVariable = BIP_ERR_TRACE(myErrorValue);                                    \
            BSTD_UNUSED(errorVariable);                                                     \
          goto errorLabel;                                                                  \
        }                                                                                   \
    } while(0)

/* BIP_CHECK_LOGERR is similar to BIP_CHECK_GOTO, but doesn't "goto". */
#define BIP_CHECK_LOGERR(                                                                   \
    expression,     /* Condition expected to be true for success.               */          \
    errorMessage,   /* If expression false, print this with BDBG_ERR.           */          \
    errorValue,     /* If expression false, call BIP_ERR_TRACE with this value. */          \
    errorVariable   /* If expression false, set this to errorValue              */          \
    )                                                                                       \
    do {                                                                                    \
        if (!(expression)) {                                                                \
            BIP_Status  myErrorValue = errorValue; /* in case errorValue refers to errno */ \
            BDBG_ERR(errorMessage);                                                         \
            errorVariable = BIP_ERR_TRACE(myErrorValue);                                    \
        }                                                                                   \
    } while(0)

/* BIP_LOGERR is similar to BIP_CHECK_LOGERR, but doesn't "check" (just logs the error messages). */
#define BIP_LOGERR(                                                                         \
    errorMessage,   /* If expression false, print this with BDBG_ERR.           */          \
    errorValue      /* If expression false, set errorVariable to this value.    */          \
    )                                                                                       \
    do {                                                                                    \
        BIP_Status  myErrorValue = errorValue; /* in case errorValue refers to errno */     \
        BDBG_ERR(errorMessage);                                                             \
        BIP_ERR_TRACE(myErrorValue);                                                        \
    } while(0)

#define BIP_CHECK_ERR_NZ_GOTO(rc, errString, errorLabel)        \
    do {                                                        \
        if ((rc) != 0) {                                        \
            BDBG_ERR(("%s: %s ", BSTD_FUNCTION, errString));     \
          goto errorLabel;                                      \
        }                                                       \
    } while(0)

/* check ptr == NULL, print errString, goto errorLable */
#define BIP_CHECK_PTR_GOTO(ptr, errString, errorLabel, error)   \
    do {                                                        \
        if (ptr == NULL) {                                      \
            BDBG_ERR(("%s: %s", BSTD_FUNCTION, errString));      \
            BIP_ERR_TRACE(error);                               \
            goto errorLabel;                                    \
        }                                                       \
    } while(0)

/* check  value < 0, print errString, goto errorLable */
#define BIP_CHECK_ERR_LZ_GOTO(rc, errString, errString2, errorLabel, error)   \
    do {                                                                      \
        if ((rc) < 0) {                                                       \
            BDBG_ERR(("%s: %s %s", BSTD_FUNCTION, errString, errString2));     \
            BIP_ERR_TRACE(error);                                             \
          goto errorLabel;                                                    \
        }                                                                     \
    } while(0)

/* check  value <= 0, print errString, goto errorLable */
#define BIP_CHECK_ERR_LEZ_GOTO(rc, errString, errString2, errorLabel, error)  \
    do {                                                                      \
        if ((rc) <= 0) {                                                      \
            BDBG_ERR(("%s: %s %s", BSTD_FUNCTION, errString, errString2));     \
            BIP_ERR_TRACE(error);                                             \
          goto errorLabel;                                                    \
        }                                                                     \
    } while(0)

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_STATUS_H */
