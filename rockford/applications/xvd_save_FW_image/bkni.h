/***************************************************************************
 *     Copyright (c) 2002-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: bkni.h $
 * $brcm_Revision: Hydra_Software_Devel/1 $
 * $brcm_Date: 2/11/11 3:38p $
 *
 * Module Description:
 *
 * THIS IS A STUB of bkni.h. This is only used for the xvd_save_FW_Image
 * program.
 *
 * Revision History:
 *
 * $brcm_Log: /rockford/applications/xvd_save_FW_image/bkni.h $
 *
 * Hydra_Software_Devel/1   2/11/11 3:38p davidp
 * SW7422-22: Initial checkin for xvd_save_image FW signing tool.
 *
 ****************************************************************************/
#ifndef BKNI_H__
#define BKNI_H__

/***************************************************************************
Summary:
    Set byte array to a value.

Description:
    Copies the value of ch (converted to an unsigned char) into each of the first n
    characters of the memory pointed to by mem.

    Can be called from an interrupt-context.

Input:
    mem - memory to be set
    ch - 8 bit value to be copied into memory
    n - number of bytes to be copied into memory

Returns:
    The value of mem
****************************************************************************/

void *BKNI_Memset(void *mem, int ch, size_t n);


/***************************************************************************
Summary:
    Copy non-overlapping memory.

Description:
    Copies n characters from the object pointed to by src into the object pointed
    to by dest.

    If copying takes place between objects that overlap, the
    behavior is undefined. Use BKNI_Memmove instead.

    Can be called from an interrupt-context.

Input:
    dest - the destination byte array
    src - the source byte array
    n - number of bytes to copy

Returns:
    The value of dest
****************************************************************************/
void *BKNI_Memcpy(void *dest, const void *src, size_t n);


/***************************************************************************
Summary:
    Compare two blocks of memory.

Description:
    Compares the first n characters of the object pointed to by s1 to the first n
    characters of the object pointed to by s2.

    Can be called from an interrupt-context.

Returns:
    An integer greater than, equal to, or less than zero, accordingly as the object
    pointed to by s1 is greater than, equal to, or less than the object pointed to by s2.
****************************************************************************/
int BKNI_Memcmp(
    const void *s1,     /* byte array to be compared */
    const void *s2,     /* byte array to be compared */
    size_t n            /* maximum number of bytes to be compared */
    );


/***************************************************************************
Summary:
    Find a byte in a block of memory.

Description:
    Locates the first occurrence of ch (converted to an unsigned char) in the initial n
    characters (each interpreted as unsigned char) of the object pointed to by mem.

    Can be called from an interrupt-context.

Input:
    mem - byte array to be searched
    ch - 8 bit value to be searched for
    n - maximum number of bytes to be searched

Returns:
    A pointer to the located character, or a null pointer if the character does not
    occur in the object.
****************************************************************************/
void *BKNI_Memchr(const void *mem, int ch, size_t n);


/***************************************************************************
Summary:
    Copy potentially overlapping memory.

Description:
    Copies n characters from the object pointed to by src into the object pointed
    to by dest. Copying takes place as if the n characters from the object pointed
    to by src are first copied into a temporary array of n characters that does
    not overlap the objects pointed to by dest and src, and then the n characters
    from the temporary array are copied into the object pointed to by dest.

    If the memory does not overlap, BKNI_Memcpy is preferred.

    Can be called from an interrupt-context.

Returns:
    The value of dest
****************************************************************************/
void *BKNI_Memmove(
    void *dest,         /* destination byte array */
    const void *src,    /* source byte array */
    size_t n            /* number of bytes to copy */
    );


/***************************************************************************
Summary:
    Print characters to the console.

Description:
    Although printing to the console is very important for development, it cannot
    and should not be guaranteed to actually print in all contexts.
    It is valid for the system developer to eliminate all BKNI_Printf output in
    release builds or if the context demands it (e.g. interrupt context).

    You should use BKNI_Printf instead of
    DebugInterface when you explicity want to print information to a console
    regardless of debug state (e.g. BXPT_PrintStatus, BPSI_PrintPsiInformation).
    BKNI_Printf is also used by the generic DebugInterface implementation.

    We only guarantee a subset of ANSI C format specifiers. These include:

    * %d  - int in decimal form
    * %u  - unsigned int in decimal form
    * %ld - long in decimal form
    * %lu - unsigned long in decimal form
    * %x  - unsigned int in lowercase hex form
    * %lx - unsigned long in lowercase hex form
    * %X  - unsigned int in uppercase hex form
    * %lX - unsigned long in uppercase hex form
    * %c  - an int argument converted to unsigned char
    * %s  - string
    * \n  - newline
    * \t  - tab
    * %%  - % character
    * %03d - Zero padding of integers, where '3' and 'd' are only examples. This can be applied to any of the preceding numeric format specifiers (not %c or %s).
    * Pass-through of non-control characters.

    Beyond these, we do not guarantee the output format.

    For BKNI_Printf and BKNI_Vprintf, other ANSI C format specifiers
    may be used, and platforms should try to make sure that any combination of formats
    and parameters will not cause a system crash.

    When calling BKNI_Snprint and BKNI_Vsnprintf, Magnum code must only use the
    guaranteed format specifiers if the results must always be the same on all platforms.

    BKNI_Printf can be called from an interrupt-context.

Returns:
    >=0 is success. It is the number of characters transmitted.
    <0 is failure, either in encoding or in outputing.
****************************************************************************/
int BKNI_Printf(
    const char *fmt, /* format string */
    ...                 /* variable arguments */
    );


/***************************************************************************
Summary:
    Print characters to a null-terminated string.

Description:
    See BKNI_Printf for a description of the format specifiers supported.

    Can be called from an interrupt-context.

Returns:
    If the output is not truncated, it returns the number of characters printed, not
    including the trailing null byte.

    If the output is truncated, it should try to return the number
    of characters that would have been printed had the size of memory been large
    enough. However, this result is not required and no Magnum code should
    depend on this result.
****************************************************************************/
int BKNI_Snprintf(
    char *s,            /* destination string */
    size_t n,           /* size of memory that can be used. It should include
                            space for the trailing null byte. */
    const char *fmt,    /* format string */
    ...                 /* variable arguments */
    );


/***************************************************************************
Summary:
    Print characters to the console using a variable argument list.

Description:
    Equivalent to BKNI_Printf, with the variable argument list replaced by the va_list
    parameter. va_list must initialized by the va_start macro (and possibly
    subsequent va_arg calls). The BKNI_Vprintf function does not invoke the va_end macro.

    The value of the va_list parameter may be modified and so it is indeterminate upon return.

    See BKNI_Printf for a description of the format specifiers supported.

    Can be called from an interrupt-context.

Input:
    fmt - See BKNI_Printf
    va_list - See StandardTypes and stdarg.h

Returns:
    >=0 is success. It is the number of characters transmitted.
    <0 is failure, either in encoding or in outputing.
****************************************************************************/
int BKNI_Vprintf(const char *fmt, va_list ap);


/***************************************************************************
Summary:
    Print characters to a null-terminated string using a variable argument list.

Description:
    See BKNI_Printf for a description of the format specifiers supported.
    See BKNI_Vprintf for a description of the va_list parameter.

    Can be called from an interrupt-context.

Input:
    s - memory to print into
    n - size of memory that can be used. It should include space for the trailing null byte.
    fmt - See BKNI_Printf
    va_list - See StandardTypes and stdarg.h

Returns:
    If the output is not truncated, it returns the number of characters printed, not
    including the trailing null byte.

    If the output is truncated, it should try to return the number
    of characters that would have been printed had the size of memory been large
    enough. However, this result is not required and no Magnum code should
    depend on this result.
****************************************************************************/
int BKNI_Vsnprintf(char *s, size_t n, const char *fmt, va_list ap);


/***************************************************************************
Summary:
    Busy sleep.

Description:
    BKNI_Delay is a busy sleep which guarantees you will delay for at least the specified
    number of microseconds. It does not call the scheduler, therefore the Delay is able to be
    less than the system clock time. This consumes CPU time, so it should be used for only
    short sleeps and only when BKNI_Sleep cannot be used.

    Be aware that on a preemptive system, any task can be interrupted and the scheduler can
    run, and so there is no guarantee of maximum delay time. If you have maximum time
    constraints, you should be using an interrupt.

    Can be called from an interrupt-context.

Input:
    microsec - minimum number of microseconds to delay

Returns:
    <none>
****************************************************************************/
void BKNI_Delay(unsigned int microsec);

void *BKNI_Malloc(
    size_t size             /* Number of bytes to allocate */
    );

/***************************************************************************
Summary:
    Dellocate system memory.

Description:
    Causes the memory pointed to by mem to be deallocated, that is, made available for
    further allocation.

    The following scenarios are not allowed and lead to undefined behavior:

    * Passing a pointer which was not returned by an earlier BKNI_Malloc call
    * Passing a pointer which was already freed
    * Passing NULL

Returns:
    <none>
****************************************************************************/

void BKNI_Free(
    void *mem           /* Pointer to memory allocated by BKNI_Malloc */
    );


/***************************************************************************
Summary:
    Yield the current task to the scheduler.

Description:
    BKNI_Sleep is a sheduler sleep which guarantees you will delay for at least the
    specified number of milliseconds. It puts the process to sleep and allows the scheduler
    to run. The minimum sleep time is dependent on the system clock time. If you need
    a minimum time which is less that the system clock time, you'll need to use BKNI_Delay.

    Actual sleep time is dependent on the scheduler but will be at least as long as
    the value specified by the millisec parameter.

    A sleep value of 0 should cause the scheduler to execute. This may or may not result in
    any delay.

    BKNI_Sleep cannot be called from an interrupt context. Use BKNI_Delay instead.

Returns:
    BERR_SUCCESS - The system slept for at least the specified number of milliseconds.
    BERR_OS_ERROR - The sleep was interrupted before the specified time.
****************************************************************************/
BERR_Code BKNI_Sleep(
    unsigned int millisec   /* minimum number of milliseconds to sleep */
    );

#endif /* BKNI_H__ */
