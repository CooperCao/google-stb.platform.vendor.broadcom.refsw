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

#ifndef BDBG_H
#define BDBG_H

#include "blst_slist.h"

#ifdef __cplusplus
extern "C" {
#endif

/*================== Module Overview =====================================
This Debug Interface provides console output according to pre-defined levels
with compile-time and run-time control.

Defined macros:
 o BDBG_MODULE - register module and allow using the rest of the BDBG_ macros.
 o BDBG_MSG, BDBG_WRN, BDBG_ERR - message output with 'Message', 'Warning' and 'Error' levels.
 o BDBG_ASSERT - evaluate expression (condition) and fail if it's false.

To activate debug build BDBG_DEBUG_BUILD macro should be defined to 1,
i.e. -DBDBG_DEBUG_BUILD=1 .

Implementation note:
During impementation of this interface for a given plaform, it should be
noted, that debug messages shall use a different output channel than a normal
output from the system. For example in Posix like systems it would be
a good choice to use stderr for the debug messages. In such a case the
following command could be used to redirect debug messages into a file:
  application 2>debug_file

A second feature of this debug interface is the BDBG_OBJECT code. These macros provide
a means to create a unique object type, add that object to a structure, and assert
when given a pointer to an object that should be of that type but isn't.  This is
analogous to the old unix "magic" values.  A structure element would be initialized
to this unique magic value at creation time and be checked each time that structure was
used to ensure that it is a valid type.  The element would be erased when the structure
was destroyed to ensure that the element wasn't used after it had been destroyed.

Defined macros:
 o BDBG_OBJECT_ID, BDBG_OBJECT_ID_DECLARE - register object and allow using the rest of the BDBG_OBJECT_ macros.
 o BDBG_OBJECT - allocate storage in a structure for the "magic" values.
 o BDBG_OBJECT_INIT, BDBG_OBJECT_DESTROY, BDBG_OBJECT_SET, BDBG_OBJECT_UNSET - update the object.
 o BDBG_OBJECT_ASSERT - assert when the object isn't the expected type.
========================================================================*/

/* Runtime debug level */
typedef enum {
  BDBG_P_eUnknown=0, /* special level that is used to indicate that file wasn't registered */
  BDBG_eTrace, 	 /* Trace level */
  BDBG_eMsg,     /* Message level */
  BDBG_eWrn,     /* Warning level */
  BDBG_eErr,     /* Error level */
  BDBG_eLog,     /* Log level */
  BDBG_P_eLastEntry
} BDBG_Level;

typedef void *BDBG_Instance;

/***************************************************************************
Summary:
	Initialize the debug interface before being used.

Description: StandardProgrammingEnvironment
	Initializes the debug interface. You must call this before making any
	other kernel interface call.

Input:
	<none>

Returns:
	BERR_SUCCESS - The debug interface successfully initialized.
****************************************************************************/
BERR_Code BDBG_Init(void);

/***************************************************************************
Summary:
	Releases resources allocated by the debug interface.

Description:
	Cleans up the debug interface interface. No debug interface calls can be made after this.

Input:
	<none>

Returns:
	<none>
****************************************************************************/
void BDBG_Uninit(void);

typedef struct BDBG_DebugModuleFile *BDBG_pDebugModuleFile;

#define BDBG_P_FILE_INITIALIZER(module) { BDBG_P_eUnknown, BDBG_P_eUnknown, false, {NULL}, #module, NULL}

typedef enum BDBG_ModulePrintKind  {
    BDBG_ModulePrintKind_eHeader, /* header only */
    BDBG_ModulePrintKind_eBody, /* body only */
    BDBG_ModulePrintKind_eHeaderAndBody, /* entire output - header and body */
    BDBG_ModulePrintKind_eString /* just a string */
} BDBG_ModulePrintKind;

typedef struct BDBG_DebugModuleFile BDBG_DebugModuleFile;

typedef void (*BDBG_DebugModule_Print)(
        BDBG_ModulePrintKind kind, /* type of the output */
        BDBG_Level level, /* level of the debug output */
        const BDBG_DebugModuleFile *module, /* pointer to the debug module */
        const char *fmt,  /* format */
        va_list ap /* variable list of arguments */
        );

struct BDBG_DebugModuleFile {
	int8_t level; /* current level for this module */
	uint8_t module_level; /* actual level for module */
	bool module_alloc; /* true if module was instantiated by API call, e.g. BDBG_SetModuleLevel */
    BLST_S_ENTRY(BDBG_DebugModuleFile) link; /* pointer that links all modules */
    const char *name; /* name of the module */
    BDBG_DebugModule_Print module_print;
};


struct bdbg_obj
{
	const char *bdbg_obj_id;
};


#if defined BDBG_DEBUG_BUILD && BDBG_DEBUG_BUILD

#if 0
/***************************************************************************
Summary:
    Registers software for the debug interface.

Description:
    This macro is used to register software module in the system. If module is used in several
    C files each file should be registered with the same module name. Every modules that uses
    BDBG_XXX macros should use BDBG_MODULE after '#include' statement.

Example:
    #include "bstd.h"
    BDBG_MODULE(some_module)

Input:
    <none>

Returns:
    <none>
****************************************************************************/
#define BDBG_MODULE(module)

/***************************************************************************
Summary:
    Registers software for the debug interface.

Description:
    This macro is used to register another software module in the same compilation unit. If module is used in several
    C files each file should be registered with the same module name. Every modules that uses
    BDBG_MODULE_XXX macros should use BDBG_FILE_MODULE after '#include' statement.

Example:
    #include "bstd.h"
    BDBG_FILE_MODULE(another_module)

Input:
    <none>

Returns:
    <none>
****************************************************************************/
#define BDBG_FILE_MODULE(module)

/***************************************************************************
Summary:
    Prints out message.

Description:
    If either module or system debug level is set to the 'Message' level this macro will print message
    on the debug output. Message can have a printf style format string but should not include an 'end
    of line' symbol ("\n") as one will be added.

Note:
    Text will not be displayed regardless of the error level on Release builds.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel

Example:
    BDBG_MSG(("Counter value %d", counter));

Input:
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_MSG(format)

/***************************************************************************
Summary:
    Prints out message.

Description:
    If either module or system debug level is set to the 'Message' level this macro will print message
    on the debug output. Message can have a printf style format string but should not include an 'end
    of line' symbol ("\n") as one will be added.

Note:
    Text will not be displayed regardless of the error level on Release builds.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel

Example:
    BDBG_MODULE_MSG(module,("Counter value %d", counter));

Input:
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_MODULE_MSG(format)

/***************************************************************************
Summary:
    Prints out message.

Description:
    If either module or system debug level is set to the 'Message' or 'Warning' level this macro will
    print the message on the debug output. Message can have a printf style format string but should
    not include an 'end of line' symbol ("\n") as one will be added.

Note:
    Text will not be displayed regardless of the error level on Release builds.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel

Example:
    BDBG_WRN(("Counter value %d greater then %d", counter, 10));

Input:
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_WRN(format)

/***************************************************************************
Summary:
    Prints out message.

Description:
    If either module or system debug level is set to the 'Message' or 'Warning' level this macro will
    print the message on the debug output. Message can have a printf style format string but should
    not include an 'end of line' symbol ("\n") as one will be added.

Note:
    Text will not be displayed regardless of the error level on Release builds.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel

Example:
    BDBG_MODULE_WRN(module,("Counter value %d greater then %d", counter, 10));

Input:
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_MODULE_WRN(format)



/***************************************************************************
Summary:
    Prints out message.

Description:
    This macro prints the message on the debug output. Message can have a printf style format string
    but should not include an 'end of line' symbol ("\n") as one will be added.

Note:
    Text will not be displayed regardless of the error level on Release builds.

Example:
    BDBG_ERR(("Counter value %d negative", counter));

Input:
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_ERR(format)

/***************************************************************************
Summary:
    Prints out message.

Description:
    This macro prints the message on the debug output. Message can have a printf style format string
    but should not include an 'end of line' symbol ("\n") as one will be added.

Note:
    Text will not be displayed regardless of the error level on Release builds.

Example:
    BDBG_ERR(module,("Counter value %d negative", counter));

Input:
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_MODULE_ERR(module,format)


/***************************************************************************
Summary:
    Test conditions.

Description:
    Evaluates expression and if result is false, prints out message including
    textual presentation of the expression, file name and line number where assert
    occured and then call function BKNI_Fail. The exact behaviour of fail is
    implementation specific. Usually it traps to the debugger, generate stack trace and/or core dump.

Note:
    Expression inside BDBG_ASSERT does not evaluated for release builds, therefore expressions
    in the BDBG_ASSERT macro shall be free of side effects.

See Also:
    BKNI_Fail, BDBG_CASSERT

Example:
    BDBG_ASSERT(counter>0);

Input:
    expr - the expression to evaluate

Returns:
    <none>
****************************************************************************/
#define BDBG_ASSERT(expr)


/***************************************************************************
Summary:
    Test conditions at compile time.

Description:
    Evaluates expression at compile time and if result is false, compile process terminates.
    The exact behaviour of compile error is implementation specific, usually it causes the compiler
    to generate an error about duplicate case values.

Note:
    Expression inside BDBG_CASSERT shall be reducable to the boolean constant at compile time
    in the BDBG_ASSER macro shall be free of side effects.

See Also:
    BKNI_ASSERT
    BDBG_CWARNING

Example:
    BDBG_CASSERT(sizeof(int)==4); // compilation fails if size of the 'int' type if different from 4

Input:
    expr - the expression to evaluate

Returns:
    <none>
****************************************************************************/
#define BDBG_CASSERT(expr)

/***************************************************************************
Summary:
    Test conditions at compile time.

Description:
    Evaluates expression at compile time and if result is false, compile process generates a warning.
    The exact behaviour of compile warning is implementation specific, usually it causes the compiler
    to generate an warning about division to zero.

Note:
    Expression inside BDBG_CASSERT shall be reducable to the boolean constant at compile time
    in the BDBG_ASSER macro shall be free of side effects.

See Also:
    BDBG_CASSERT

Example:
    BDBG_WARNING(sizeof(int)==4); // compilation fails if size of the 'int' type if different from 4

Input:
    expr - the expression to evaluate

Returns:
    <none>
****************************************************************************/
#define BDBG_CWARNING(expr)


/***************************************************************************
Summary:
    Prints out message.

Description:
    If either instance, module, or system debug level is set to the 'Message' level this macro will print
    the message on the debug output. The message can have a printf style format string but should not
    include and 'end of line' character ("\n") as one will be added.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel, BDBG_SetInstanceLevel

Example:
    BDBG_INSTANCE_MSG(obj, ("Obj's counter value %d", obj->counter));

Input:
    instance - the module instance
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_INSTANCE_MSG(instance, format)


/***************************************************************************
Summary:
    Prints out message.

Description:
    If either instance, module, or system debug level is set to the 'Message' or 'Warning' level this macro
    will print the message on the debug output. The message can have a printf style format string but should
    not include an 'end of line' character ("\n") as one will be added.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel, BDBG_SetInstanceLevel

Example:
    BDBG_INSTANCE_WRN(obj, ("Counter value %d greater then %d", obj->counter, 10));

Input:
    instance - the module instance
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_INSTANCE_WRN(instance, format)


/***************************************************************************
Summary:
    Prints out message.

Description:
    This macro prints the message on the debug output. Message can have a printf style format string
    but should not include an 'end of line' character ("\n") as one will be added.

Example:
    BDBG_INSTANCE_ERR(obj, ("Counter value %d negative", obj->counter));

Input:
    instance - the module instance
    format - message to print

Returns:
    <none>
****************************************************************************/
#define BDBG_INSTANCE_ERR(instance, format)


/***************************************************************************
Summary:
    Prints out message.

Description:
    If either module or system debug level is set to the 'Trace' level this macro will print the
    message that function 'function' entered.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel

Example:
    BDBG_ENTER(BFOO_DoBar);

Input:
    function - function name

Returns:
    <none>
****************************************************************************/
#define BDBG_ENTER(function)


/***************************************************************************
Summary:
    Prints out message.

Description:
    If either module or system debug level is set to the 'Trace' level this macro will print the
    message that function 'function' exited.

See Also:
    BDBG_SetLevel, BDBG_SetModuleLevel

Example:
    BDBG_LEAVE(BFOO_DoBar);

Input:
    function - function name

Returns:
    <none>
****************************************************************************/
#define BDBG_LEAVE(function)


/***************************************************************************
Summary:
    Registers instance

Description:
    This macro registers an intance for the debug interface. An 'Instance' is any pointer like object.
    Only registered instances should be passed to other debug interface API.

See Also:
    BDBG_SetInstanceLevel, BDBG_INSTANCE_MSG, BDBG_INSTANCE_WRN, BDBG_INSTANCE_ERR

Example:
    BDBG_REGISTER_INSTANCE(obj);

Input:
    instance - the module instance

Returns:
    BERR_SUCCESS - instance has been registered
****************************************************************************/
#define BDBG_REGISTER_INSTANCE(instance)


/***************************************************************************
Summary:
    Unregisters instance

Description:
    This macro releases resources allocated for this instance.

See Also:
    BDBG_REGISTER_INSTANCE

Example:
    BDBG_UNREGISTER_INSTANCE(obj);

Input:
    instance - the module instance

Returns:
    BERR_SUCCESS - instance has been unregistered
****************************************************************************/
#define BDBG_UNREGISTER_INSTANCE(instance)

/***************************************************************************
Summary:
    Releases resources

Description:
    This macro releases resources allocated for the file(module) which used this macro.

Example:
    BDBG_RELEASE();

****************************************************************************/
#define BDBG_RELEASE()


/***************************************************************************
Summary:
    Creates object type id

Description:
    This macro creates the object type id, that to use to check the object type.
    This macro shall be used in the 'C' file.
    Only one id with the given object type (name) can exist in the system.

Example:
    // this shows a non-opaque struct object_type.
    // if struct object_type is private (e.g. opaque handle), then all this code should be in the implementation file.

    // header file
    BDBG_OBJECT_ID_DECLARE(object_type);
    struct object_type {
        BDBG_OBJECT(object_type)
        unsigned x;
    }

    // implementation file
    BDBG_OBJECT_ID(object_type);

    object_type *open_object() {
        object_type *obj = malloc(sizeof(*obj));
        memset(obj, 0, sizeof(*obj));
        BDBG_OBJECT_SET(obj, object_type);
        obj->x = 1;
        return obj;
    }
    void use_object(object_type *obj) {
        BDBG_OBJECT_ASSERT(obj, object_type);
        obj->x = 2;
    }
    void close_object(object_type *obj) {
        BDBG_OBJECT_ASSERT(obj, object_type);
        obj->x = 3;
        BDBG_OBJECT_DESTROY(obj, object_type);
        free(obj);
    }

See Also:
    BDBG_OBJECT_INIT, BDBG_OBJECT_ASSERT

Input:
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_ID(name)


/***************************************************************************
Summary:
    Declares object type id

Description:
    This macro forward declares the object type id.
    Usually this is used in an H file if its necessary to share the same object ID
    across multiple C files.

Example:
    see example for BDBG_OBJECT_ID

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_INIT, BDBG_OBJECT_ASSERT

Input:
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_ID_DECLARE(name)


/***************************************************************************
Summary:
    Adds module id to structure

Description:
    This macro is used to add module id to a structure.
    This needs to be initialized or set before it can be used in an assert test.

Note:
    A semi-colon should not be used with this macro as some compilers will generate
    an error when creating a release build and the macro is removed.

Example:
    see example for BDBG_OBJECT_ID

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_INIT, BDBG_OBJECT_SET, BDBG_OBJECT_ASSERT

Input:
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT(name)


/***************************************************************************
Summary:
    Initializes object

Description:
    This macro is used to initialize pointer and sets the module id.
    Initialization code fills memory, equal to size of the object, with a random pattern.

Example:
    see example for BDBG_OBJECT_ID. compare with:

    object_type *open_object() {
        object_type *obj = malloc(sizeof(*obj));
        BDBG_OBJECT_INIT(obj, object_type);
        obj->x = 1;
        return obj;
    }

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_DESTROY, BDBG_OBJECT_ASSERT

Input:
    ptr - pointer to the object
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_INIT(ptr,name)


/***************************************************************************
Summary:
    Destroys object

Description:
    This macro is used to destroy object and remove the type tag. Using this
    macro fills memory, equal to size of the object, with a random pattern.
    The assert test will fail if pointer to object is used after its destroyed.

Example:
    see example for BDBG_OBJECT_ID

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_INIT, BDBG_OBJECT_ASSERT

Input:
    ptr - pointer to the object
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_DESTROY(ptr,name)


/***************************************************************************
Summary:
    Sets the object tag

Description:
    This macro is used tag pointer with a specific tag.

Example:
    see example for BDBG_OBJECT_ID

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_UNSET, BDBG_OBJECT_ASSERT

Input:
    ptr - pointer to the object
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_SET(ptr,name)


/***************************************************************************
Summary:
    Removes the object tag

Description:
    This macro is used remove tag from the pointer.

Example:
    see example for BDBG_OBJECT_ID
    BDBG_OBJECT_UNSET is an alternative to BDBG_OBJECT_DESTROY.

    BDBG_OBJECT_ID(object_type);
    struct some *p = malloc(sizeof(struct some));
    BDBG_OBJECT_UNSET(p, object_type);

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_INIT, BDBG_OBJECT_SET, BDBG_OBJECT_ASSERT

Input:
    ptr - pointer to the object
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_UNSET(ptr,name)


/***************************************************************************
Summary:
    Validates a pointer and type of the pointer

Description:
    This macro is used validates that pointer is tagged with a specific object type.
    The pointer must be valid in order for this macro to succeed. If pointer doesn't
    match the expected type, this macro will cause a software assert.

Example:
    see example for BDBG_OBJECT_ID

See Also:
    BDBG_OBJECT_ID, BDBG_OBJECT_INIT, BDBG_OBJECT_SET

Input:
    ptr - pointer to the object
    name - object type

Returns:
    <none>
****************************************************************************/
#define BDBG_OBJECT_ASSERT(ptr,name)

/***************************************************************************
Summary:
    Format string to print unsigned 64-bit number

Description:
    This macro is used in conjunction with BDBG_UINT64_ARG to safely print
    64-bit number.

Example:
    BDBG_MSG(("%u number foo64=" BDBG_UINT64_FMT " bar32=%u", i, BDBG_UINT64_ARG(foo64), bar32));


See Also:
    BDBG_UINT64_ARG
****************************************************************************/
#define BDBG_UINT64_FMT

/***************************************************************************
Summary:
    Argument processing to print unsigned 64-bit number

Description:
    This macro is used in conjunction with BDBG_UINT64_FMT to safely print
    64-bit number.

Example:
    see example for BDBG_UINT64_FMT

See Also:
    BDBG_UINT64_FMT
****************************************************************************/
#define BDBG_UINT64_ARG(x)


#define BDBG_OBJECT_INIT_INST(ptr,name,inst)
#define BDBG_OBJECT_SET_INST(ptr,name,inst)
#define BDBG_OBJECT_TEST_INST(ptr,name,inst)

#endif /* 0 */


#ifdef __GNUC__
#ifdef B_REFSW_STRICT_PRINTF_FORMAT
#define BDBG_P_PRINTF_FORMAT(fmt,args) __attribute__((format (printf, fmt, args)))
#else
#define BDBG_P_PRINTF_FORMAT(fmt,args)
#endif
#define BDBG_MODULE(module) static BDBG_DebugModuleFile __attribute__ ((__unused__)) b_dbg_module = BDBG_P_FILE_INITIALIZER(module)
#define BDBG_FILE_MODULE(module) static BDBG_DebugModuleFile __attribute__ ((__unused__)) b_dbg_module_##module = BDBG_P_FILE_INITIALIZER(module)
#else
#define BDBG_P_PRINTF_FORMAT(fmt,args)
#define BDBG_MODULE(module) static BDBG_DebugModuleFile b_dbg_module = BDBG_P_FILE_INITIALIZER(module)
#define BDBG_FILE_MODULE(module) static BDBG_DebugModuleFile b_dbg_module_##module = BDBG_P_FILE_INITIALIZER(module)
#endif

#if defined(__STDC_VERSION__)
# if __STDC_VERSION__ == 199901L || defined BDBG_ANDROID_LOG
#  define BDBG_P_UNWRAP(...) __VA_ARGS__
# endif
#endif
#if !defined(BDBG_P_UNWRAP) && defined(BDBG_USE_VA_ARGS)
# define BDBG_P_UNWRAP(args...) args
#endif

#if defined(BDBG_P_UNWRAP)
# define BDBG_P_PRINTMSG_PRIV(module,lvl, fmt) (((lvl) >= module.level)? (void)BDBG_P_TestAndPrint_isrsafe((lvl), &module, BDBG_P_UNWRAP fmt) : (void)0)
# define BDBG_P_PRINTMSG_COMPACT_PRIV(module, lvl, fmt) (void)BDBG_P_TestAndPrint_##lvl##_isrsafe(&module, BDBG_P_UNWRAP fmt)
# define BDBG_P_PRINTMSG_VERBOSE_PRIV(module, lvl, _file, _line_num, fmt) ((void)BDBG_P_TestAndPrint_##lvl##_isrsafe(&module, BDBG_P_UNWRAP fmt), \
                                                                            BDBG_P_PrintString(">>>>>>>>>>>>>>>> %s(%d)", _file, _line_num))
# define BDBG_P_INSTANCE_PRINTMSG_PRIV(module, lvl, instance, fmt) (void)BDBG_P_InstTestAndPrint_isrsafe((lvl), &module, (instance), BDBG_P_UNWRAP fmt)
#else
# define BDBG_P_PRINTMSG_PRIV(module, lvl, fmt) ((((lvl) >= module.level) && BDBG_P_TestAndPrint_isrsafe((lvl), &module, NULL)) ? BDBG_P_PrintWithNewLine_isrsafe fmt:(void)0)
# define BDBG_P_PRINTMSG_COMPACT_PRIV(module, lvl, fmt) (BDBG_P_TestAndPrint_##lvl##_isrsafe(&module, NULL) ? BDBG_P_PrintWithNewLine_isrsafe fmt:(void)0)
# define BDBG_P_PRINTMSG_VERBOSE_PRIV(module, lvl, _file, _line_num, fmt) ((BDBG_P_TestAndPrint_##lvl##_isrsafe(&module, NULL)) ? \
                                                                             (BDBG_P_PrintWithNewLine_isrsafe fmt ,\
                                                                              BDBG_P_PrintString(">>>>>>>>>>>>>>>> %s(%d)", _file, _line_num)) : (void)0)
# define BDBG_P_INSTANCE_PRINTMSG_PRIV(module, lvl, instance, fmt) ((BDBG_P_InstTestAndPrint_isrsafe((lvl), &module, (instance), NULL))? BDBG_P_PrintWithNewLine_isrsafe fmt:(void)0)
#endif

#define BDBG_P_PRINTMSG(lvl, fmt) BDBG_P_PRINTMSG_PRIV(b_dbg_module, lvl, fmt)
#define BDBG_P_PRINTMSG_COMPACT(lvl, fmt) BDBG_P_PRINTMSG_COMPACT_PRIV(b_dbg_module, lvl, fmt)
#define BDBG_P_PRINTMSG_VERBOSE(lvl, _file, _line_num, fmt) BDBG_P_PRINTMSG_VERBOSE_PRIV(b_dbg_module, lvl, _file, _line_num, fmt)
#define BDBG_P_MODULE_PRINTMSG(module, lvl, fmt) BDBG_P_PRINTMSG_PRIV(b_dbg_module_##module, lvl, fmt)
#define BDBG_P_MODULE_PRINTMSG_COMPACT(module, lvl, fmt) BDBG_P_PRINTMSG_COMPACT_PRIV(b_dbg_module_##module, lvl, fmt)
#define BDBG_P_INSTANCE_PRINTMSG(lvl, instance, fmt) BDBG_P_INSTANCE_PRINTMSG_PRIV(b_dbg_module, lvl, instance, fmt)
#define BDBG_P_MODULE_INSTANCE_PRINTMSG(module, lvl, instance, fmt) BDBG_P_INSTANCE_PRINTMSG_PRIV(b_dbg_module_##module, lvl, instance, fmt)

#define BDBG_REGISTER_INSTANCE(handle) BDBG_P_RegisterInstance(handle, &b_dbg_module)
#define BDBG_UNREGISTER_INSTANCE(handle) BDBG_P_UnRegisterInstance(handle, &b_dbg_module)
#define BDBG_RELEASE() BDBG_P_Release(&b_dbg_module)

#define BDBG_MODULE_REGISTER_INSTANCE(module, handle) BDBG_P_RegisterInstance(handle, &b_dbg_module_##module)
#define BDBG_MODULE_UNREGISTER_INSTANCE(module, handle) BDBG_P_UnRegisterInstance(handle, &b_dbg_module_##module)
#define BDBG_MODULE_RELEASE(module) BDBG_P_Release(&b_dbg_module__#module)

#if defined B_REFSW_DEBUG_COMPACT_ERR || defined B_REFSW_STATIC_ANALYZER
#define BDBG_ASSERT(expr) BDBG_P_Assert_isrsafe(expr, BSTD_FILE, BSTD_LINE)
#else
#define BDBG_ASSERT(expr) (expr) ? (void) 0 : BDBG_P_AssertFailed(#expr, BSTD_FILE, BSTD_LINE)
#endif
#define BDBG_CASSERT(expr) do switch(0){case 0: case (expr):;} while(0)
#define BDBG_CWARNING(expr) do {if(0){int unused = 1/(expr);unused++;}} while(0)
#define BDBG_CWARNING_EXPR(expr) (1/(expr) ? 0 : 0)


#define BDBG_OBJECT_ID(name) const char bdbg_id__##name[]= "#" #name
#define BDBG_OBJECT_ID_DECLARE(name) extern const char bdbg_id__##name[]
#define BDBG_OBJECT(name) struct bdbg_obj bdbg_object_##name;
#define BDBG_OBJECT_INIT(ptr,name) BDBG_Object_Init((ptr),sizeof(*(ptr)),&(ptr)->bdbg_object_##name, bdbg_id__##name)
#define BDBG_OBJECT_DESTROY(ptr,name) BDBG_Object_Init((ptr),sizeof(*(ptr)),&(ptr)->bdbg_object_##name, NULL)


#define BDBG_OBJECT_SET(ptr,name) (ptr)->bdbg_object_##name.bdbg_obj_id=bdbg_id__##name
#define BDBG_OBJECT_UNSET(ptr,name) (ptr)->bdbg_object_##name.bdbg_obj_id=NULL

void BDBG_Object_Assert_isrsafe(const void *ptr, size_t size, const struct bdbg_obj *obj, const char *id, const char *file, unsigned line);
#ifdef B_REFSW_DEBUG_COMPACT_ERR
#define BDBG_OBJECT_ASSERT(ptr,name) BDBG_Object_Assert_isrsafe(ptr, sizeof(*ptr), &(ptr)->bdbg_object_##name, bdbg_id__##name, BSTD_FILE, BSTD_LINE)
#else
#define BDBG_OBJECT_ASSERT(ptr,name) (((ptr) && (ptr)->bdbg_object_##name.bdbg_obj_id==bdbg_id__##name)? (void) 0 : BDBG_Object_Assert_isrsafe(ptr, sizeof(*ptr), &(ptr)->bdbg_object_##name, bdbg_id__##name, BSTD_FILE, BSTD_LINE))
#endif

#define BDBG_OBJECT_INIT_INST(ptr,name,inst) BDBG_Object_Init(ptr,sizeof(*(ptr)),&(ptr)->bdbg_object_##name,bdbg_id__##name+(unsigned)(inst))
#define BDBG_OBJECT_SET_INST(ptr,name,inst) (ptr)->bdbg_object_##name.bdbg_obj_id=(bdbg_id__##name + (unsigned)(inst))
#define BDBG_OBJECT_ASSERT_INST(ptr,name,inst) BDBG_ASSERT((ptr)->bdbg_object_##name.bdbg_obj_id==&bdbg_id__##name+(unsigned)(inst))

void BDBG_Object_Init(void *ptr, size_t size, struct bdbg_obj *obj, const char *id);

bool BDBG_P_TestAndPrint_BDBG_eWrn_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(2, 3);
bool BDBG_P_TestAndPrint_BDBG_eErr_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(2, 3);
bool BDBG_P_TestAndPrint_BDBG_eLog_isrsafe(BDBG_pDebugModuleFile dbg_module, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(2, 3);
bool BDBG_P_TestAndPrint_isrsafe(BDBG_Level level, BDBG_pDebugModuleFile dbg_module, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(3, 4);
bool BDBG_P_InstTestAndPrint_isrsafe(BDBG_Level level, BDBG_pDebugModuleFile dbg_module, BDBG_Instance handle, const char *fmt, ...) BDBG_P_PRINTF_FORMAT(4, 5);
void BDBG_P_RegisterInstance(BDBG_Instance handle, BDBG_pDebugModuleFile dbg_module);
void BDBG_P_UnRegisterInstance(BDBG_Instance handle, BDBG_pDebugModuleFile dbg_module);
void BDBG_P_PrintWithNewLine_isrsafe(const char *fmt, ...) BDBG_P_PRINTF_FORMAT(1, 2);
void BDBG_P_Release(BDBG_pDebugModuleFile dbg_module);

void BDBG_EnterFunction(BDBG_pDebugModuleFile dbg_module, const char *function);
void BDBG_LeaveFunction(BDBG_pDebugModuleFile dbg_module, const char *function);


#else /* BDBG_DEBUG_BUILD */
/* stubs */

#define BDBG_MODULE(module) extern int bdbg_unused
#define BDBG_FILE_MODULE(module) extern int bdbg_unused

#define BDBG_RELEASE() BDBG_NOP()
#define BDBG_MODULE_RELEASE(module) BDBG_NOP()

#define BDBG_REGISTER_INSTANCE(instance)
#define BDBG_MODULE_REGISTER_INSTANCE(module,instance)
#define BDBG_UNREGISTER_INSTANCE(instance)
#define BDBG_MODULE_UNREGISTER_INSTANCE(module,instance)

#define BDBG_ASSERT(expr) BDBG_NOP()
#define BDBG_CASSERT(expr) do switch(0){case 0: case (expr):;} while(0)
#define BDBG_CWARNING(expr) do {if(0){int unused = 1/(expr);unused++;}} while(0)
#define BDBG_CWARNING_EXPR(expr) (1/(expr) ? 0 : 0)


#define BDBG_OBJECT_ID(name) extern const char bdbg_id_unused_##name
#define BDBG_OBJECT_ID_DECLARE(name) extern const char bdbg_id_unused_decl_##name
#define BDBG_OBJECT(name)
#define BDBG_OBJECT_INIT(ptr,name) (void)ptr
#define BDBG_OBJECT_DESTROY(ptr,name) (void)ptr
#define BDBG_OBJECT_SET(ptr,name) (void)ptr
#define BDBG_OBJECT_UNSET(ptr,name) (void)ptr
#define BDBG_OBJECT_ASSERT(ptr,name)  (void)ptr
#define BDBG_OBJECT_INIT_INST(ptr,name,inst) (void)ptr
#define BDBG_OBJECT_SET_INST(ptr,name,inst) (void)ptr
#define BDBG_OBJECT_TEST_INST(ptr,name,inst) (void)ptr
#define BDBG_OBJECT_ASSERT_INST(ptr,name,inst) (void)ptr

/* if not BDBG_DEBUG_BUILD, force these macros to be defined. 
they can be used to avoid unused code warnings. */
#ifndef BDBG_NO_MSG
#define BDBG_NO_MSG 1
#endif
#ifndef BDBG_NO_WRN
#define BDBG_NO_WRN 1
#endif
#ifndef BDBG_NO_LOG
#define BDBG_NO_LOG 1
#endif
#if !defined BDBG_NO_ERR && !defined B_REFSW_DEBUG_COMPACT_ERR
#define BDBG_NO_ERR 1
#endif
#endif /* BDBG_DEBUG_BUILD */

#ifdef BDBG_NO_MSG
#define BDBG_ENTER(function) BDBG_NOP()
#define BDBG_ENTER_F() BDBG_NOP()
#define BDBG_MODULE_ENTER(module,function) BDBG_NOP()
#define BDBG_LEAVE(function) BDBG_NOP()
#define BDBG_LEAVE_F() BDBG_NOP()
#define BDBG_MODULE_LEAVE(module,function) BDBG_NOP()
#define BDBG_MSG(format) BDBG_NOP()
#define BDBG_MODULE_MSG(module, format) BDBG_NOP()
#define BDBG_INSTANCE_MSG(instance, format) BDBG_NOP()
#define BDBG_MODULE_INSTANCE_MSG(module, instance, format) BDBG_NOP()
#else
#define BDBG_ENTER(function) ((BDBG_eTrace >= b_dbg_module.level)? BDBG_EnterFunction(&b_dbg_module, #function)  : (void)0)
#define BDBG_ENTER_F() ((BDBG_eTrace >= b_dbg_module.level)? BDBG_EnterFunction(&b_dbg_module, __FUNCTION__)  : (void)0)
#define BDBG_MODULE_ENTER(module, function) ((BDBG_eTrace >= b_dbg_module_##module.level)? BDBG_EnterFunction(&b_dbg_module_##module, #function)  : (void)0)
#define BDBG_LEAVE(function) ((BDBG_eTrace >= b_dbg_module.level)? BDBG_LeaveFunction(&b_dbg_module, #function)  : (void)0)
#define BDBG_LEAVE_F() ((BDBG_eTrace >= b_dbg_module.level)? BDBG_LeaveFunction(&b_dbg_module, __FUNCTION__)  : (void)0)
#define BDBG_MODULE_LEAVE(module, function) ((BDBG_eTrace >= b_dbg_module_##module.level)? BDBG_LeaveFunction(&b_dbg_module_##module, #function)  : (void)0)
#define BDBG_MSG(format) BDBG_P_PRINTMSG(BDBG_eMsg, format)
#define BDBG_MODULE_MSG(module, format) BDBG_P_MODULE_PRINTMSG(module, BDBG_eMsg, format)
#define BDBG_INSTANCE_MSG(instance, format) BDBG_P_INSTANCE_PRINTMSG(BDBG_eMsg, instance, format)
#define BDBG_MODULE_INSTANCE_MSG(module, instance, format) BDBG_P_MODULE_INSTANCE_PRINTMSG(module, BDBG_eMsg, instance, format)
#endif

#ifdef BDBG_NO_WRN
#define BDBG_WRN(format) BDBG_NOP()
#define BDBG_MODULE_WRN(module,format) BDBG_NOP()
#define BDBG_INSTANCE_WRN(instance, format) BDBG_NOP()
#define BDBG_MODULE_INSTANCE_WRN(module,instance, format) BDBG_NOP()
#else
#define BDBG_WRN(format) BDBG_P_PRINTMSG_COMPACT(BDBG_eWrn, format)
#define BDBG_MODULE_WRN(module,format) BDBG_P_MODULE_PRINTMSG_COMPACT(module, BDBG_eWrn, format)
#define BDBG_INSTANCE_WRN(instance, format) BDBG_P_INSTANCE_PRINTMSG(BDBG_eWrn, instance, format)
#define BDBG_MODULE_INSTANCE_WRN(module, instance, format) BDBG_P_MODULE_INSTANCE_PRINTMSG(module, BDBG_eWrn, instance, format)
#endif

#ifdef BDBG_NO_ERR
#define BDBG_ERR(format) BDBG_NOP()
#define BDBG_MODULE_ERR(module,format) BDBG_NOP()
#define BDBG_INSTANCE_ERR(instance, format) BDBG_NOP()
#define BDBG_MODULE_INSTANCE_ERR(module, instance, format) BDBG_NOP()
#elif defined B_REFSW_DEBUG_COMPACT_ERR
#define BDBG_ERR(format)                                   BDBG_P_PrintErrorString_small_isrsafe(BSTD_FILE, BSTD_LINE)
#define BDBG_MODULE_ERR(module,format)                     BDBG_ERR(format)
#define BDBG_INSTANCE_ERR(instance, format)                BDBG_ERR(format)
#define BDBG_MODULE_INSTANCE_ERR(module, instance, format) BDBG_ERR(format)
#else
#ifdef B_REFSW_DEBUG_VERBOSE_ERR
#define BDBG_ERR(format) BDBG_P_PRINTMSG_VERBOSE(BDBG_eErr, __FILE__, __LINE__, format)
#else
#define BDBG_ERR(format) BDBG_P_PRINTMSG_COMPACT(BDBG_eErr, format)
#endif
#define BDBG_MODULE_ERR(module,format) BDBG_P_MODULE_PRINTMSG_COMPACT(module,BDBG_eErr, format)
#define BDBG_INSTANCE_ERR(instance, format) BDBG_P_INSTANCE_PRINTMSG(BDBG_eErr, instance, format)
#define BDBG_MODULE_INSTANCE_ERR(module, instance, format) BDBG_P_MODULE_INSTANCE_PRINTMSG(module,BDBG_eErr, instance, format)
#endif

#ifdef BDBG_NO_LOG
#define BDBG_LOG(format) BDBG_NOP()
#define BDBG_MODULE_LOG(module,format) BDBG_NOP()
#define BDBG_INSTANCE_LOG(instance, format) BDBG_NOP()
#define BDBG_MODULE_INSTANCE_LOG(module, instance, format) BDBG_NOP()
#else
#define BDBG_LOG(format) BDBG_P_PRINTMSG_COMPACT(BDBG_eLog, format)
#define BDBG_MODULE_LOG(module,format) BDBG_P_MODULE_PRINTMSG_COMPACT(module,BDBG_eLog, format)
#define BDBG_INSTANCE_LOG(instance, format) BDBG_P_INSTANCE_PRINTMSG(BDBG_eLog, instance, format)
#define BDBG_MODULE_INSTANCE_LOG(module, instance, format) BDBG_P_MODULE_INSTANCE_PRINTMSG(module,BDBG_eLog, instance, format)
#endif

#if defined BDBG_DEBUG_BUILD && BDBG_DEBUG_BUILD && !defined(BDBG_DEBUG_WITH_STRINGS)
#define BDBG_DEBUG_WITH_STRINGS 1
#endif
extern const char BDBG_P_EmptyString[];
#ifdef BDBG_DEBUG_WITH_STRINGS
#define BDBG_STRING(X) X
#define BDBG_STRING_INLINE(X) X
#else
#define BDBG_STRING(X) BDBG_P_EmptyString
#define BDBG_STRING_INLINE(X) ""
#endif

/***************************************************************************
Summary:
    Prepares filename to be included into the debug output

Description:
    This function substitutes NULL with an empty string, and for not NULL inputs
    it reduces length of pathname.

Example:
    BDBG_GetPrintableFileName(__FILE__);

Input:
    pFileName - filename as instantiated by __FILE__ define

Returns:
    Pointer to string, it's never NULL
****************************************************************************/
const char *
BDBG_GetPrintableFileName(const char *pFileName);

#define BDBG_UINT64_FMT "0x%x%08x"
#define BDBG_UINT64_ARG(x) (unsigned)((x)>>32), (unsigned)(x)

const char *BDBG_P_Int64DecArg(int64_t x, char *buf, size_t buf_size);
#define BDBG_INT64_DEC_FMT  "%s"

#if defined BDBG_DEBUG_BUILD && BDBG_DEBUG_BUILD
#define BDBG_INT64_DEC_BUF(buf) char _bdbg_int64_buf_##buf[16]
#define BDBG_INT64_DEC_ARG(buf,x) BDBG_P_Int64DecArg((x), _bdbg_int64_buf_##buf, sizeof(_bdbg_int64_buf_##buf))
#else
#define BDBG_INT64_DEC_BUF(buf)
#define BDBG_INT64_DEC_ARG(buf,x) ""
#endif


#ifdef __cplusplus
}
#endif
#include  "bdbg_app.h"
#include  "bdbg_priv.h"

#endif  /* BDBG_H */
