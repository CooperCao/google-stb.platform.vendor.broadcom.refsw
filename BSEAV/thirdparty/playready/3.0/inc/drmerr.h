/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __DRMERR_H__
#define __DRMERR_H__

#include <drmtypes.h>
#include <drmdebug.h>
#include <drmresults.h>
#include <drmpragmas.h> /* Required to ChkDR( constant ); */

#if DRM_DBG

ENTER_PK_NAMESPACE
    extern void (*g_pfDebugAnalyzeDR)(unsigned long, const char*, unsigned long, const char*);
EXIT_PK_NAMESPACE

#define SetDbgAnalyzeFunction(pfn) g_pfDebugAnalyzeDR = pfn;
#define GetDbgAnalyzeFunction() g_pfDebugAnalyzeDR

#define ExamineDRValue(_drval_,_file_,_line_,_expr_) DRM_DO {                                               \
        if( g_pfDebugAnalyzeDR != NULL )                                                                    \
        {                                                                                                   \
            (*g_pfDebugAnalyzeDR)((unsigned long)(_drval_), (_file_), (unsigned long)(_line_), (_expr_));   \
        }                                                                                                   \
    } DRM_WHILE_FALSE

#else  /* DRM_DBG */

#define SetDbgAnalyzeFunction(pfn)
#define ExamineDRValue(_drval_,_file_,_line_,_expr_)
#define GetDbgAnalyzeFunction()

#endif  /* DRM_DBG */

#if defined (__GNUC__)

/* If using the GNU compiler use __builtin_expect to improve branch predictions */
#define DRM_LIKELY( expr ) __builtin_expect(!!(expr),1)
#define DRM_UNLIKELY( expr ) __builtin_expect(!!(expr),0)

#else  /* __GNUC__ */

#define DRM_LIKELY( expr )    (expr)
#define DRM_UNLIKELY( expr )  (expr)

#endif  /* __GNUC__ */

#define ChkDRAllowENOTIMPL(expr) DRM_DO {                   \
        dr = (expr);                                        \
        if( dr == DRM_E_NOTIMPL )                           \
        {                                                   \
            dr = DRM_SUCCESS;                               \
        }                                                   \
        else                                                \
        {                                                   \
            ExamineDRValue(dr, __FILE__, __LINE__, #expr);  \
            if( DRM_FAILED( dr ) )                          \
            {                                               \
                goto ErrorExit;                             \
            }                                               \
        }                                                   \
    } DRM_WHILE_FALSE

#define ChkDRNoGOTO(expr) DRM_DO {                          \
            dr = (expr);                                    \
            ExamineDRValue(dr, __FILE__, __LINE__, #expr);  \
    } DRM_WHILE_FALSE

#define ChkDR(expr) DRM_DO {                                \
            ChkDRNoGOTO( expr );                            \
            if( DRM_FAILED( dr ) )                          \
            {                                               \
                goto ErrorExit;                             \
            }                                               \
        } DRM_WHILE_FALSE

#define ChkMem(expr) DRM_DO {                                                                   \
            if( DRM_UNLIKELY( NULL == (expr) ) )                                                \
            {                                                                                   \
                DRM_DBG_TRACE( ("Allocation failure at %s : %d.\n%s\n", __FILE__, __LINE__, #expr) );   \
                dr = DRM_E_OUTOFMEMORY;                                                         \
                ExamineDRValue(dr, __FILE__, __LINE__, #expr);                                  \
                goto ErrorExit;                                                                 \
            }                                                                                   \
        } DRM_WHILE_FALSE

#define ChkArgError() DRM_DO {                                                                  \
            DRM_DBG_TRACE( ("Invalid argument at %s : %d.\nFALSE\n", __FILE__, __LINE__) );             \
            dr = DRM_E_INVALIDARG;                                                              \
            ExamineDRValue(dr, __FILE__, __LINE__, "FALSE");                                    \
            goto ErrorExit;                                                                     \
        } DRM_WHILE_FALSE

#define ChkArg(expr) DRM_DO {                                                                   \
            if( DRM_UNLIKELY( !(expr) ) )                                                       \
            {                                                                                   \
                DRM_DBG_TRACE( ("Invalid argument at %s : %d.\n%s\n", __FILE__, __LINE__, #expr) );     \
                dr = DRM_E_INVALIDARG;                                                          \
                ExamineDRValue(dr, __FILE__, __LINE__, #expr);                                  \
                goto ErrorExit;                                                                 \
            }                                                                                   \
        } DRM_WHILE_FALSE

#define ChkPtr(expr) DRM_DO {                                                                   \
            if( DRM_UNLIKELY( NULL == (expr) ) )                                                \
            {                                                                                   \
                DRM_DBG_TRACE( ("NULL pointer at %s : %d.\n%s\n", __FILE__, __LINE__, #expr) );         \
                dr = DRM_E_POINTER;                                                             \
                ExamineDRValue(dr, __FILE__, __LINE__, #expr);                                  \
                goto ErrorExit;                                                                 \
            }                                                                                   \
        } DRM_WHILE_FALSE

#define ChkDRMString(s) DRM_DO {                                                                \
            if( !(s) || (s)->pwszString == NULL || (s)->cchString == 0 )                        \
            {                                                                                   \
                DRM_DBG_TRACE( ("Invalid argument at %s : %d.\n%s\n", __FILE__, __LINE__, #s) );        \
                dr = DRM_E_INVALIDARG;                                                          \
                ExamineDRValue(dr, __FILE__, __LINE__, #s);                                     \
                goto ErrorExit;                                                                 \
            }                                                                                   \
        } DRM_WHILE_FALSE

#define ChkDRMANSIString(s) DRM_DO {                                                            \
            if( !(s) || (s)->pszString == NULL || (s)->cchString == 0 )                         \
            {                                                                                   \
                DRM_DBG_TRACE( ("Invalid argument at %s : %d.\n%s\n", __FILE__, __LINE__, #s) );        \
                dr = DRM_E_INVALIDARG;                                                          \
                ExamineDRValue(dr, __FILE__, __LINE__, #s);                                     \
                goto ErrorExit;                                                                 \
            }                                                                                   \
        } DRM_WHILE_FALSE

#define ChkBOOL(fExpr,err) DRM_DO {                             \
            if( !(fExpr) )                                      \
            {                                                   \
                dr = (err);                                     \
                ExamineDRValue(dr, __FILE__, __LINE__, #fExpr); \
                goto ErrorExit;                                 \
            }                                                   \
        } DRM_WHILE_FALSE

#define ChkVOID(fExpr) DRM_DO {         \
            fExpr;                      \
        } DRM_WHILE_FALSE

#define ChkDRContinue(exp) DRM_DO {                         \
            dr=(exp);                                       \
            ExamineDRValue(dr, __FILE__, __LINE__, #exp);   \
            if( DRM_FAILED( dr ) )                          \
            {                                               \
                continue;                                   \
            }                                               \
        } DRM_WHILE_FALSE

#define ChkDRMap( expr, drOriginal, drMapped ) DRM_DO {         \
            dr = ( expr );                                      \
            ExamineDRValue(dr, __FILE__, __LINE__, #expr);      \
            if( dr == ( drOriginal ) )                          \
            {                                                   \
                dr = ( drMapped );                              \
                ExamineDRValue(dr, __FILE__, __LINE__, #expr);  \
            }                                                   \
            if( DRM_FAILED( dr ) )                              \
            {                                                   \
                goto ErrorExit;                                 \
            }                                                   \
        } DRM_WHILE_FALSE

#define MapDR( drOriginal, drMapped ) DRM_DO {                                                                      \
            DRM_DBG_TRACE( ("Error code 0x%X mapped at %s : %d. to 0x%X \n", drOriginal,  __FILE__, __LINE__, drMapped) );  \
            drOriginal = ( drMapped );                                                                              \
        } DRM_WHILE_FALSE

#define AssertLogicError() DRM_DO {     \
        DRMASSERT( FALSE );             \
        dr = DRM_E_LOGICERR;            \
        goto ErrorExit;                 \
    } DRM_WHILE_FALSE

#define AssertChkBOOL(expr) DRM_DO {    \
        DRM_BOOL _f = (expr);           \
        DRMASSERT( _f );                \
        if( DRM_UNLIKELY( !_f ) )       \
        {                               \
            dr = DRM_E_LOGICERR;        \
            goto ErrorExit;             \
        }                               \
        __analysis_assume( expr );      \
    } DRM_WHILE_FALSE

#define AssertChkArg(expr) AssertChkBOOL(expr)

#define AssertChkFeature(expr) DRM_DO {                                                                     \
        DRM_BOOL _f = (expr);                                                                               \
        if( DRM_UNLIKELY( !_f ) )                                                                           \
        {                                                                                                   \
            DRM_DBG_TRACE( ( "Incompatible Feature Set Detected at %s : %d.\n%s\n", __FILE__, __LINE__, #expr ) );  \
            DRMASSERT( FALSE );                                                                             \
            ChkDR( DRM_E_BCERT_INVALID_FEATURE );                                                           \
        }                                                                                                   \
    } DRM_WHILE_FALSE

#define DRM_REQUIRE_BUFFER_TOO_SMALL( drRet )   DRM_DO {   \
        DRM_RESULT __drTemp = (drRet);                     \
        if( __drTemp != DRM_E_BUFFERTOOSMALL )             \
        {                                                  \
            ChkDR( __drTemp );                             \
            DRMASSERT( FALSE );                            \
            ChkDR( DRM_E_LOGICERR );                       \
        }                                                  \
    } DRM_WHILE_FALSE

#define InitOutputPtr( lhs, rhs ) DRM_DO {  \
        if( (lhs) != NULL )                 \
        {                                   \
            *(lhs) = (rhs);                 \
        }                                   \
    } DRM_WHILE_FALSE

#define ChkAndInitOutputPtr( lhs, rhs ) DRM_DO {    \
        ChkArg( (lhs) != NULL );                    \
        *(lhs) = (rhs);                             \
    } DRM_WHILE_FALSE

#endif /* __DRMERR_H__ */
