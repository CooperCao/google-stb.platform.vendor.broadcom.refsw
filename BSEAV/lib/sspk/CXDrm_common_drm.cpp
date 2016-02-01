///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// Implementation of IXDrm for LinuxSample

// NOTE: A Linux-compatible version of WMDRM-MD headers is required for compilation 
//       and a Linux build of a WMDRM-MD library must be available for linking.

#include "pkPAL.h"
#include "pkExecutive.h"

#include "strsafe.h"

#include <new>
#include <map>
#include <string>


#include "IPTVDecoderHal.h"

#include "AutoLock.h"

#include "CXHttp.h"

#include "CXDrm.h"
#include "nexus_memory.h"

#include "drm_prdy.h"
#include "bkni.h"

#ifdef DEBUG
#define CXDRM_TRACE_ENABLE
#endif

#if defined(CXDRM_TRACE_ENABLE)
#define CXDRM_LOG_MESSAGE( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define CXDRM_LOG_MESSAGE( printf_exp )   ( void ) 0
#endif

#if defined(CXDRM_TRACE_VERBOSE)
#define CXDRM_LOG_MESSAGE_VERBOSE( printf_exp ) ( Executive_DebugPrintf printf_exp )
#else
#define CXDRM_LOG_MESSAGE_VERBOSE( printf_exp )   ( void ) 0
#endif

static const uint32_t DRM_TRANSACTION_LOOP_DELAY = 200;

static const uint32_t MAXIMUM_LICACQ_RESPONSE_BODY_SIZE = 1000000;

#define CXDRM_LICENSE_STORAGE_FILE "sample.hds"

static inline IXDRM_HRESULT DRM_Prdy_Err_To_XDRM_HRESULT(DRM_Prdy_Error_e prdy_err)
{
    IXDRM_HRESULT dr;
    switch(prdy_err){
        case DRM_Prdy_ok:
            dr = pkS_OK;
            break;
        case DRM_Prdy_fail:
            dr = pkE_FAIL;
            break;
        case DRM_Prdy_buffer_size:
            dr = pkE_INSUFFICIENT_BUFFER;
            break;
        case DRM_Prdy_invalid_parameter:
            dr = pkE_INVALIDARG;
            break;
        case DRM_Prdy_license_not_found:
        case DRM_Prdy_license_expired:
        case DRM_Prdy_domain_join:
        case DRM_Prdy_revocation_package_expired:
        case DRM_Prdy_invalid_header:
        case DRM_Prdy_xml_not_found:
        case DRM_Prdy_header_not_set:
        default:
            dr = pkE_FAIL;
            break;
    }
    return dr;
}

// ================================================================

struct CompareDRMID
{
    bool operator() ( const DRM_Prdy_Id_t &lhs, const DRM_Prdy_Id_t &rhs ) const
    { 
        return memcmp( &lhs, &rhs, sizeof( DRM_Prdy_Id_t ) ) < 0; 
    }
};

typedef std::map<DRM_Prdy_Id_t, DRM_Prdy_DecryptContext_t*, CompareDRMID> DecryptorMap;

// singleton Decryptor map 
static DecryptorMap g_oDecryptorMap;

const DRM_Prdy_guid_t ANALOG_VIDEO_AGC_OUTPUT_ID              = { 0xC3FD11C6, 0xF8B7, 0x4D20, {0xB0, 0x08, 0x1D, 0xB1, 0x7D, 0x61, 0xF2, 0xDA}};
const DRM_Prdy_guid_t ANALOG_VIDEO_EXPLICIT_OUTPUT_ID         = { 0x2098DE8D, 0x7DDD, 0x4BAB, {0x96, 0xC6, 0x32, 0xEB, 0xB6, 0xFA, 0xBE, 0xA3}};
const DRM_Prdy_guid_t BEST_EFFORT_CGMSA_ID                    = { 0x225CD36F, 0xF132, 0x49EF, {0xBA, 0x8C, 0xC9, 0x1E, 0xA2, 0x8E, 0x43, 0x69}};
const DRM_Prdy_guid_t ANALOG_VIDEO_COMPONENT_OUTPUT_ID        = { 0x811C5110, 0x46C8, 0x4C6E, {0x81, 0x63, 0xC0, 0x48, 0x2A, 0x15, 0xD4, 0x7E}};
const DRM_Prdy_guid_t ANALOG_VIDEO_COMPUTER_MONITOR_OUTPUT_ID = { 0xD783A191, 0xE083, 0x4BAF, {0xB2, 0xDA, 0xE6, 0x9F, 0x91, 0x0B, 0x37, 0x72}};
const DRM_Prdy_guid_t DIGITIAL_AUDIO_SCMS_ID                  = { 0x6D5CFA59, 0xC250, 0x4426, {0x93, 0x0E, 0xFA, 0xC7, 0x2C, 0x8F, 0xCF, 0xA6}};


extern "C" const char g_rchUrlDefault[] = "http://go.microsoft.com/fwlink/?LinkID=59833";

// ================================================================
// Class statics
// ================================================================

// lock for method operation:
Lockable CXDrm::s_OperationLock;

// Singleton operation:
Lockable CXDrm::s_FactoryLock;
int CXDrm::s_nRefCount = 0;
IXDrm* CXDrm::s_poXDrm = NULL;

//
// Factory method that creates an IXDrm instance. XDrm is implemented
// as a singleton so if it has already been created before, a reference
// to the extsing instance is returned and the reference count is
// incremented.
//
//
IXDRM_HRESULT CXDrm::_CreateInstance( IXDrm **ppXDrm )
{
    AutoLock lock(&s_FactoryLock);
    
    if ( s_poXDrm == NULL )
    {
        s_poXDrm = NEW_NO_THROW CXDrm;
        if (NULL == s_poXDrm)
        {
            return (IXDRM_HRESULT) pkE_UNEXPECTED;
        }
    }

    s_nRefCount++;
    *ppXDrm = s_poXDrm;
    
    return (IXDRM_HRESULT) pkS_OK;
}

//
// Destroy an IXDrm instance. First the reference count is decremented
// and if is reaches 0, the singleton XDrm instance is destroyed,
// otherwise the singleton XDrm instance is left alone.
//
IXDRM_HRESULT CXDrm::_DestroyInstance( IXDrm *pXDrm )
{
    AutoLock lock(&s_FactoryLock);
    
    if( pXDrm == NULL || pXDrm != s_poXDrm )
    {
        return (IXDRM_HRESULT) pkE_UNEXPECTED;
    }

    s_nRefCount--;
    if ( s_nRefCount == 0 )
    {
        delete s_poXDrm;
        s_poXDrm = NULL;
    }

    return (IXDRM_HRESULT) pkS_OK;
}

// ================================================================
// Private methods
// ================================================================

// Constructor/destructor

CXDrm::CXDrm() 
    : m_poPrdyContext( 0 )
    , m_pfnXDrmOPLCallback( NULL )
    , m_pvXDrmOPLCallbackContext( NULL )
    , m_fInTransaction(false)
    , m_fInit(false)
    , m_fInitDRM(false)
{ 
    CXDRM_LOG_MESSAGE( ("CXDrm::CXDrm object CREATED\n" ) );
}

CXDrm::~CXDrm() 
{ 
    int nDecryptorsFreed = g_oDecryptorMap.size();
    
    if ( m_poPrdyContext )
    {
        DRM_Prdy_Uninitialize( m_poPrdyContext );
    }

    DecryptorMap::iterator it;
    for ( it = g_oDecryptorMap.begin(); it != g_oDecryptorMap.end(); ++it )
    {
        DRM_Prdy_Reader_Close(it->second);
        BKNI_Free( it->second );
    }

    g_oDecryptorMap.clear();

    CXDRM_LOG_MESSAGE( ("CXDrm::~CXDrm Decryptor count = %d\n", nDecryptorsFreed) );
}

// Initialize a XDrm instance.
// This performs all initialization EXCEPT Drm_Initialize
// which happens in _InitDRMIfRequired.  This ensures
// that Drm_Initialize, which may end up forcing Activation,
// never happens on a UI thread.
//
// Arguments:   none.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::_Init()
{
    IXDRM_HRESULT dr = pkS_OK;

    // Allocate/initialize the necessary buffers and data structures.

ErrorExit:
    return dr;
}

//
// Initialize PlayReady DRM
// If Drm_Initialize has not yet been called, this function
// calls Drm_Initialize and Drm_Revocation_SetBuffer.
// Otherwise, it does nothing.
// If Drm_Initialize fails because Activation is required,
// synchronously performs Activation and then calls Drm_Initialize again.
//
// Preconditions:
//    Must be called within the operation lock
//
// Arguments:
// [f_pfJustInitialized] On output:
//                       Set to true if Drm_Initialize was called successfully
//                       regardless of whether Activation was performed or not.
//                       Set to false otherwise.
//
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::_InitDRMIfRequired( bool *f_pfJustInitialized )
{
    IXDRM_HRESULT dr = pkS_OK;
    
    *f_pfJustInitialized = false;

    if( !m_fInitDRM )
    {
        DRM_Prdy_Init_t initSettings;

        DRM_Prdy_GetDefaultParamSettings(&initSettings);
        initSettings.hdsFileName = (char *)CXDRM_LICENSE_STORAGE_FILE;

        m_poPrdyContext = DRM_Prdy_Initialize(&initSettings);
        if(m_poPrdyContext == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }

        m_fInitDRM = true;
        *f_pfJustInitialized = true;
    }

ErrorExit:

    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::_InitDRMIfRequired failed = 0x%x\n", dr ) );
    }    
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::_InitDRMIfRequired succeeded; %s initialized\n", *f_pfJustInitialized ? "Just" : "Already" ) );
    }

    return dr;
}

// ================================================================
// IXDrm implementation methods
// ================================================================

//
// Initialize a XDrm instance. Call Drm_Reinitialize if the instance has
// already been initialized before.
//
// Arguments:
// [pfnCallback]    Policy callback.
// [pfnCallbackArg] Parameter to be passed to the policy callback.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::Init( XDrmOPLCallbackPtr pfnCallback, void* pvCallbackContext )
{
    IXDRM_HRESULT dr = pkS_OK;
    
    AutoLock lock(&s_OperationLock);

    if ( !m_fInit )
    {
        dr = _Init();
        if(!dr)
        {
            goto ErrorExit;
        }

        m_fInit = true;
    }

    //
    // This function gets called a LOT with NULL callbacks,
    // but we only care about the first valid callback it's called with.
    // However, it is also repeatedly called with different callback args
    // as the key rotates and different CPlayReadyLicense objects are created.
    // Thus, the arg changes even as the callback remains the same.
    //
    if( pfnCallback != NULL )
    {
        if( m_pfnXDrmOPLCallback != NULL )
        {
            //
            // Callback is immutable once set
            //
            pkASSERT( m_pfnXDrmOPLCallback == pfnCallback );
        }
        else
        {
            m_pfnXDrmOPLCallback = pfnCallback;
        }
        m_pvXDrmOPLCallbackContext = pvCallbackContext;
    }
    else
    {
        //
        // Argument should always be null if callback is null
        //
        pkASSERT( pvCallbackContext == NULL );
    }

ErrorExit:
    
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Init failed = 0x%x\n", dr ) );
    }    
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Init succeeded\n") );
    }
    return dr;
}


IXDRM_HRESULT CXDrm::_DrmPolicy(
    IN const DRM_Prdy_policy_t        *f_pPolicy
    )
{
    IXDRM_HRESULT dr = pkS_OK;
    void *pvCallbackData = NULL;

    if ( f_pPolicy->type == PLAY_OPL )
    {
        XDRM_OPL_DATA *pOPLData = NULL;

        pOPLData = ( XDRM_OPL_DATA * )BKNI_Malloc( sizeof( XDRM_OPL_DATA ) );
        if(pOPLData == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }

        BKNI_Memset( pOPLData, 0, sizeof( XDRM_OPL_DATA ) );

        pvCallbackData = pOPLData;

        DRM_Prdy_opl_play_ex2_t *pOPL = ( DRM_Prdy_opl_play_ex2_t * )&f_pPolicy->t.play;
        uint16_t wUncompressedVideoOPL = pOPL->minOPL.wUncompressedDigitalVideo;

        if ( wUncompressedVideoOPL <= 250 )
        {
            // No restriction.
        }
        else if ( wUncompressedVideoOPL <= 270 )
        {
            // Allow output to external display but the content has to down-res.
            pOPLData->HDCPAction = XDRM_OPL_ENABLE_DOWN_RES;
        }
        else
        {
            // Does not allow output to external display.
            pOPLData->HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
        }
        
        // Check for explicit analog video/audio protections.
        // There are two categories of analog video/audio protections: One does not allow output if there
        // is an external display and the other down-res when there is an external display.
        // Since the behavior is the same as that of HDCP OPLs we are reusing HDCP bits to achieve the same goal. 
        for ( int i = 0; i < pOPL->vopi.cEntries; i++ )
        {
            if ( memcmp( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_COMPONENT_OUTPUT_ID, sizeof( DRM_Prdy_guid_t ) ) == 0 ||
                 memcmp( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_COMPUTER_MONITOR_OUTPUT_ID, sizeof( DRM_Prdy_guid_t ) ) == 0 )
            {
                // Allow output to external display but the content has to down-res.
                pOPLData->HDCPAction = XDRM_OPL_ENABLE_DOWN_RES;
            }
            else if ( memcmp( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_AGC_OUTPUT_ID, sizeof( DRM_Prdy_guid_t ) ) == 0 ||
                      memcmp( &pOPL->vopi.rgVop[ i ].guidId, &ANALOG_VIDEO_EXPLICIT_OUTPUT_ID, sizeof( DRM_Prdy_guid_t ) ) == 0 )
            {
                // Does not allow output to external display.
                pOPLData->HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
            }
            else if ( memcmp( &pOPL->vopi.rgVop[ i ].guidId, &BEST_EFFORT_CGMSA_ID, sizeof( DRM_Prdy_guid_t ) ) == 0 )
            {
                // Do nothing for "Best effort".
            }
        }

        for ( int i = 0; i < pOPL->aopi.cEntries; i++ )
        {
            if ( memcmp( &pOPL->aopi.rgAop[ i ].guidId, &DIGITIAL_AUDIO_SCMS_ID, sizeof( DRM_Prdy_guid_t ) ) == 0 )
            {
                // Does not allow output to external display.
                pOPLData->HDCPAction = XDRM_OPL_ENABLE_ALWAYS;
            }
        }

        dr = m_pfnXDrmOPLCallback( pOPLData, m_pvXDrmOPLCallbackContext );
        if(!dr) goto ErrorExit;
    }
    else
    {
        dr = pkE_NOTIMPL;
        goto ErrorExit;
    }

ErrorExit:
    BKNI_Free( pvCallbackData );
    return dr;
}

//
// Set the rights to be performed.
//
// Arguments:
// [ulRights]   The rights to be performed.
//
// Returns:
// DRM_Prdy_ok.
//
IXDRM_HRESULT CXDrm::SetRights(
    unsigned long ulRights )
{
    // Always succeed.
    return( DRM_Prdy_ok );
}

//
// Set the current DRM header.
//
// Arguments:
// [cbHdr]      Size of the DRM header.
// [pbHdr]      Pointer to a buffer containing the DRM header.
// [cbKeyID]    Size of an optional key id.
// [pbKeyID]    (Optional) pointer to a buffer containing the key id.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::SetEnhancedData(
        size_t cbHdr,
        const uint8_t* pbHdr,
        size_t cbKeyID,
        const uint8_t* pbKeyID ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    DRM_Prdy_Error_e prdy_dr = DRM_Prdy_ok;

    bool fJustInitialized = false;
    PRDY_DRM_WCHAR *pwchB64KeyID     = NULL;
    
    CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData cbHdr %u, pbHdr %p, cbKeyID %u, pbKeyID %p\n", 
        cbHdr, pbHdr, cbKeyID, pbKeyID) );
    
    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    dr =  _InitDRMIfRequired( &fJustInitialized );
    if(dr != DRM_Prdy_ok) goto ErrorExit;

    if ( pbKeyID != NULL && cbKeyID > 0 )
    {
        DRM_Prdy_Content_Set_Property_Obj_With_KID_Data_t oData = { 0, 0, NULL, 0 };

        uint32_t cchB64KeyID = DRM_Prdy_Cch_Base64_Equiv( cbKeyID );
        pwchB64KeyID = ( PRDY_DRM_WCHAR * )BKNI_Malloc( cchB64KeyID * sizeof( PRDY_DRM_WCHAR ) );
        if(pwchB64KeyID == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }

        prdy_dr =  DRM_Prdy_B64_EncodeW((uint8_t *) pbKeyID,
                                cbKeyID,
                                pwchB64KeyID,
                                &cchB64KeyID);
        if(prdy_dr != DRM_Prdy_ok)
        {
            CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData DRM_Prdy_B64_EncodeW failed = 0x%x\n", prdy_dr ) );
            dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        }

        oData.pbKeyID = ( uint8_t * )pwchB64KeyID;
        oData.cbKeyID = cchB64KeyID * sizeof( PRDY_DRM_WCHAR );
        oData.pbHeaderData = pbHdr;
        oData.cbHeaderData = cbHdr;

        
        prdy_dr = DRM_Prdy_Content_SetProperty(
                    m_poPrdyContext,
                    DRM_Prdy_contentSetProperty_ePlayreadyObjWithKID,
                    ( uint8_t * )&oData,
                    sizeof( oData ) );
        if(prdy_dr != DRM_Prdy_ok)
        {
            dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
            goto ErrorExit;
        }
    }
    else
    {
        prdy_dr = DRM_Prdy_Content_SetProperty(
                    m_poPrdyContext,
                    DRM_Prdy_contentSetProperty_eAutoDetectHeader,
                    (uint8_t *)pbHdr,
                    cbHdr );
        if(prdy_dr != DRM_Prdy_ok)
        {
            dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
            goto ErrorExit;
        }
    }

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::SetEnhancedData Drm_Content_SetProperty failed = 0x%x\n", dr ) );
    }    
    return dr;
}

//
// Retrieve the decrypt context based on key id. If key id
// is NULL then a blank key id is internally used. A new
// decrypt context is allocated if currently there is not one
// that matches the requested key id.
//
// Arguments:
// [cbKeyID]    Size of a key id.
// [pbKeyID]    Pointer to a buffer containing the key id.
// [ppvDecryptContext]  Pointer to a pointer to receive the decrypt context.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::AcquireDecryptContext(
        size_t cbKeyID,
        const uint8_t* pbKeyID,
        void **ppvDecryptContext ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    
    DecryptorMap::iterator it;
    DRM_Prdy_Id_t oKID;

    AutoLock lock(&s_OperationLock);

    pkASSERT(m_fInitDRM);

    if ( pbKeyID != NULL )
    {
        if( cbKeyID != sizeof( DRM_Prdy_Id_t ))
        {
            dr = pkE_INVALIDARG;
            goto ErrorExit;
        }

        BKNI_Memcpy( &oKID, pbKeyID, cbKeyID );
    }
    else
    {
        BKNI_Memset( &oKID, 0, sizeof( DRM_Prdy_Id_t ) );
    }

    it = g_oDecryptorMap.find( oKID );

    if ( it != g_oDecryptorMap.end() )
    {
        DRM_Prdy_DecryptContext_t *poContext = it->second;
        if( poContext == NULL)
        {
            dr = pkE_FAIL;
            goto ErrorExit;
        }

        *ppvDecryptContext = poContext;
    }
    else
    {
        DRM_Prdy_DecryptContext_t *poContext = NULL;
        poContext = ( DRM_Prdy_DecryptContext_t * )BKNI_Malloc( sizeof( DRM_Prdy_DecryptContext_t ) );
        if(poContext == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }

        BKNI_Memset( poContext, 0, sizeof( DRM_Prdy_DecryptContext_t ) );
        g_oDecryptorMap.insert( std::make_pair( oKID, poContext ) );
        *ppvDecryptContext = poContext;

        if( pbKeyID )
        {
            DRM_Prdy_guid_t *pguid = (DRM_Prdy_guid_t *)pbKeyID;
            CXDRM_LOG_MESSAGE( ("CXDrm::AcquireDecryptContext KID = {%08X-%04X-%04X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X}\n",
                                  pguid->Data1, pguid->Data2, pguid->Data3,
                                  pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3], 
                                  pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7] ) );
        }
        CXDRM_LOG_MESSAGE( ("CXDrm::AcquireDecryptContext Decryptor count = %d\n", g_oDecryptorMap.size() ) );
    }

ErrorExit:
    return dr;
}

//
// Release the decrypt context related to a key id (created in
// AcquireDecryptContext).
//
// Arguments:
// [cbKeyID]    Size of a key id.
// [pbKeyID]    Pointer to a buffer containing the key id.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::ReleaseDecryptContext(
        size_t cbKeyID,
        const uint8_t* pbKeyID ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    
    DecryptorMap::iterator it;
    DRM_Prdy_Id_t oKID;
    
    AutoLock lock(&s_OperationLock);

    if ( !m_fInitDRM )
    {
        goto ErrorExit;
    }


    if ( pbKeyID != NULL )
    {
        DRM_Prdy_guid_t *pguid = (DRM_Prdy_guid_t *)pbKeyID;

        if( cbKeyID != sizeof( DRM_Prdy_Id_t ))
        {
            dr = pkE_INVALIDARG;
            goto ErrorExit;
        }

        BKNI_Memcpy( &oKID, pbKeyID, cbKeyID );

        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext KID = {%08X-%04X-%04X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X}\n",
                              pguid->Data1, pguid->Data2, pguid->Data3,
                              pguid->Data4[0], pguid->Data4[1], pguid->Data4[2], pguid->Data4[3], 
                              pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7] ) );
    }
    else
    {
        BKNI_Memset( &oKID, 0, sizeof( DRM_Prdy_Id_t ) );
    }

    it = g_oDecryptorMap.find( oKID );

    if ( it != g_oDecryptorMap.end() )
    {
        DRM_Prdy_DecryptContext_t *poContext = it->second;
        DRM_Prdy_Reader_Close(poContext);
        BKNI_Free( poContext );
        g_oDecryptorMap.erase( it );

        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext Decryptor count = %d\n", g_oDecryptorMap.size() ) );
    }
    else
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::ReleaseDecryptContext FAILED TO FIND KID [%02X%02X%02X%02X]\n", 
            oKID.rgb[3], oKID.rgb[2], oKID.rgb[1], oKID.rgb[0] ) );
    }

ErrorExit:
    return dr;
}


//
// Perform the DRM bind function to locate a license of a
// piece of content with a specific rights.
//
// Arguments:
// [pvDecryptContext]   Pointer to a decrypt context to use.
// [fAbortPlayback]     Flag indicating whether playback should be aborted if error occurrs.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::CanDecrypt( void *pvDecryptContext, bool fAbortPlayback )
{
    IXDRM_HRESULT dr = pkS_OK;
    DRM_Prdy_Error_e prdy_dr = DRM_Prdy_ok;
    DRM_Prdy_policy_t policy;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    if(pvDecryptContext == NULL )
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }

    if((prdy_dr = DRM_Prdy_Reader_Bind(m_poPrdyContext, (DRM_Prdy_DecryptContext_t *)pvDecryptContext)) != DRM_Prdy_ok)
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt Drm_Reader_Bind failed = 0x%x\n", prdy_dr ) );
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        goto ErrorExit;
    }

    /* Apply policies */
    do{ 
        if((prdy_dr = DRM_Prdy_Get_Protection_Policy(m_poPrdyContext, &policy)) == DRM_Prdy_ok)
        {
            dr = _DrmPolicy(&policy);
        }
    } while(prdy_dr == DRM_Prdy_ok);

    if(prdy_dr == DRM_Prdy_no_policy)
        dr = pkS_OK;
    else {
        CXDRM_LOG_MESSAGE( ("CXDrm::CanDecrypt DRM_Prdy_Get_Protection_Policy failed = 0x%x\n", prdy_dr ) );
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
    }

ErrorExit:
    return dr;
}


//
// Decrypt a subsample buffer.
//
// Arguments:
// [pvDecryptContext]   Pointer to a decrypt context to use.
// [pbData]     Pointer to a buffer containing the subsample data.
// [cbData]     Size of the buffer mentioned above.
// [fIsAES]     Currently it must be true.
// [qwSampleID] The Sample ID associated with the subsample.
// [qwOffset]   Offset of the subsample within the sample it belongs to.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::Decrypt(
        void *pvDecryptContext,
        uint8_t* pbData,
        size_t cbData,
        bool fIsAES,
        uint64_t qwSampleID,
        uint64_t qwOffset ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    DRM_Prdy_AES_CTR_Info_t ctrModeCtx;
    DRM_Prdy_Error_e prdy_dr = DRM_Prdy_ok;
    
    CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::Decrypt [%p] cbData: %u iv:%llu offset:%llu\n", 
        pvDecryptContext, cbData, qwSampleID, qwOffset ) );

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    if( pvDecryptContext == NULL )
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }


    if( fIsAES)
    {
        dr = pkE_FAIL;
        goto ErrorExit;
    }

    BKNI_Memcpy( &ctrModeCtx.qwInitializationVector, ( uint8_t * )&qwSampleID, sizeof( uint64_t ) );
    ctrModeCtx.qwBlockOffset = qwOffset / 16;
    ctrModeCtx.bByteOffset = qwOffset % 16;

    prdy_dr = DRM_Prdy_Reader_Decrypt((DRM_Prdy_DecryptContext_t *)pvDecryptContext,
                                      &ctrModeCtx, 
                                      pbData,
                                      cbData);
    if(prdy_dr != DRM_Prdy_ok)
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Decrypt [%p] FAILED [0x%X]\n", 
                pvDecryptContext, prdy_dr )); 
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        goto ErrorExit;
    }

ErrorExit:
    return dr;
}

//
// Decrypt a chain of subsamples. The list entry is in the form of
// IPTV_HAL_BUFFER. The implementation only uses the input list to
// decrypt the data in place.
//
// Arguments:
// [pvDecryptContext]   Pointer to a decrypt context to use.
// [pInBufList]     Input list of subsample buffers to be decrypted.
// [pOutBufList]    Output list of subsample buffers after being decrypted.
// [fIsAES]         Currently it must be true.
// [qwSampleID]     The Sample ID associated with the subsample.
// [qwOffset]       Offset of the subsample within the sample it belongs to.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::DecryptBufferChain(
        void *pvDecryptContext,
        void* pInBufList,
        void* pOutBufList,
        bool fIsAES,
        uint64_t qwSampleID,
        uint64_t qwOffset ) 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    DRM_Prdy_AES_CTR_Info_t ctrModeCtx;
    DRM_Prdy_Error_e prdy_dr = DRM_Prdy_ok;
    uint64_t qwOffset1 = qwOffset;    
    
    CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::DecryptBufferChain [%p] in:%p out:%p iv:%llu offset:%llu\n", 
        pvDecryptContext, pInBufList, pOutBufList, qwSampleID, qwOffset ) );
    
    AutoLock lock(&s_OperationLock);
    pkASSERT( m_fInitDRM );

    if( pvDecryptContext == NULL )
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }

    if(!( pInBufList != NULL && pOutBufList != NULL ))
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }

    
    if(!fIsAES)
    {
        dr = pkE_FAIL;
        goto ErrorExit;
    }

    // loop through the input buffer chain
    {
        PIPTV_HAL_BUFFER pBufListItem = (PIPTV_HAL_BUFFER) pInBufList;

        while (NULL != pBufListItem)
        {
            if ( ( ( pBufListItem->u32Flags & IPTV_HAL_BUFFER_FLAG_DECRYPT ) != 0 ) )
            {
                uint32_t cbSize;
        
                BKNI_Memcpy( &ctrModeCtx.qwInitializationVector, ( uint8_t * )&qwSampleID, sizeof( uint64_t ) );
                ctrModeCtx.qwBlockOffset = qwOffset1 / 16;
                ctrModeCtx.bByteOffset = qwOffset1 % 16;
        
                cbSize = pBufListItem->u32DataEnd - pBufListItem->u32DataStart;

                prdy_dr = DRM_Prdy_Reader_Decrypt((DRM_Prdy_DecryptContext_t *)pvDecryptContext,
                                                  &ctrModeCtx, 
                                                  pBufListItem->pBuf + pBufListItem->u32DataStart,
                                                  cbSize);
                if(prdy_dr != DRM_Prdy_ok)
                {
                    CXDRM_LOG_MESSAGE( ("CXDrm::Decrypt [%p] FAILED [0x%X]\n", 
                            pvDecryptContext, prdy_dr )); 
                    dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
                    goto ErrorExit;
                }
                qwOffset1 += cbSize;
            }
            pBufListItem = pBufListItem->pNext;
        }
    }

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::DecryptBufferChain [%p] FAILED [0x%X]\n", 
            pvDecryptContext, dr )); 
    }
    return dr;
}

//
// Commit the bind operation.
//
// Arguments:   none.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::Commit() 
{ 
    IXDRM_HRESULT dr = pkS_OK;
    DRM_Prdy_Error_e prdy_dr = DRM_Prdy_ok;

    CXDRM_LOG_MESSAGE( ("CXDrm::Commit\n" ) ); 
    
    _RequireTransaction();

    AutoLock lock(&s_OperationLock);

    pkASSERT( m_fInitDRM );

    prdy_dr = DRM_Prdy_Reader_Commit(m_poPrdyContext);
    if(prdy_dr != DRM_Prdy_ok)
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::Commit [%p] FAILED [0x%X]\n", 
            m_poPrdyContext, prdy_dr ));
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
    }

    return dr;
}

//
// Generate a license acquisition challenge. Both the generated Url and challenge strings are
// NULL terminated.  Also cleans up stale licenses.
//
// Arguments:
// [ppszUrl]    Pointer to a pointer of a buffer to store the license acquisition Url.
//              It's caller's responsibility to release the buffer via MemFree after usage.
// [pszCustomData]  Pointer to a custom data string to be sent along with the challenge.
// [ppszChallenge]  Pointer to a pointer of a buffer to store the license acquisition challenge.
//              It's caller's responsibility to release the buffer via MemFree after usage.
// [fAllowCustomDataOverride] Boolean to indicate whether to retrieve the custom data from the player. Not Used.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::GenerateChallenge(
    char** ppszUrl,
    const char* pszCustomData,
    char** ppszChallenge,
    bool fAllowCustomDataOverride )
{
    IXDRM_HRESULT dr = pkS_OK;
    uint8_t *pbChallenge = NULL;
    uint8_t *pchURL = NULL;
    uint8_t *pszCustomDataUsed = NULL;
    uint32_t cchCustomDataUsed = 0;
    uint32_t cbChallenge = 0;
    uint32_t cchURL = 0;
    DRM_Prdy_Error_e prdy_dr;

    _RequireTransaction();

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    if(!( ppszUrl != NULL && ppszChallenge != NULL ))
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }

    if ((NULL != pszCustomData) && (0 != pszCustomData[0]))
    {
        pszCustomDataUsed = ( uint8_t * )pszCustomData;
        cchCustomDataUsed = (uint32_t) strlen(pszCustomData);
    }

    prdy_dr =  DRM_Prdy_Get_Buffer_Size(
        m_poPrdyContext,
        DRM_Prdy_getBuffer_licenseAcq_challenge,
        pszCustomDataUsed, 
        cchCustomDataUsed,
        &cchURL,
        &cbChallenge);
    if ( prdy_dr != DRM_Prdy_ok )
    {
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        goto ErrorExit;
    }
    else 
    {
        pchURL = ( uint8_t * )BKNI_Malloc( cchURL + 1 );
        if(pchURL == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }


        BKNI_Memset( pchURL, 0, cchURL + 1 );

        pbChallenge = ( uint8_t * )BKNI_Malloc( cbChallenge + 1 );
        if(pbChallenge == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }

        BKNI_Memset( pbChallenge, 0, cbChallenge + 1 );

        prdy_dr  = DRM_Prdy_LicenseAcq_GenerateChallenge(
                            m_poPrdyContext,
                            (const char *)pszCustomDataUsed,
                            cchCustomDataUsed,
                            (char *)pchURL,
                            (uint32_t *)&cchURL, 
                            (char *)pbChallenge,
                            &cbChallenge); 
        if(prdy_dr != DRM_Prdy_ok)
        {
            dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
            goto ErrorExit;
        }
        pbChallenge[ cbChallenge ] = 0;
    }

    // Cleanup stale licenses.
    prdy_dr = DRM_Prdy_Cleanup_LicenseStores( m_poPrdyContext);
    if(prdy_dr != DRM_Prdy_ok)
    {
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        goto ErrorExit;
    }

    *ppszUrl = (char *)pchURL;
    pchURL = NULL;
    *ppszChallenge = (char *)pbChallenge;
    pbChallenge = NULL;

ErrorExit:
    if(pchURL != NULL) BKNI_Free( pchURL );
    if(pbChallenge != NULL) BKNI_Free( pbChallenge );
    
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::GenerateChallenge [%p] FAILED [0x%X]\n", 
            m_poPrdyContext, dr )); 
    }
    return dr;
}


//
// Process a license acquisition response.
//
// Arguments:
// [pszResponse]    Pointer to buffer containing the license acquisition response.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::ProcessResponse(
    const char* pszResponse )
{
    IXDRM_HRESULT dr = pkS_OK;
    int nLen;
    DRM_Prdy_Error_e prdy_dr = DRM_Prdy_ok;

    //
    // NOTE: This method does not use the wmrm header
    //       therefore _RequireTransaction should not be called
    //
    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    if( pszResponse == NULL )
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }

    nLen = strlen( pszResponse );

    prdy_dr = DRM_Prdy_LicenseAcq_ProcessResponse(m_poPrdyContext,
                                                  pszResponse,
                                                  nLen, NULL);
    if(prdy_dr != DRM_Prdy_ok)
    {
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        goto ErrorExit;
    }

ErrorExit:
    if (pkFAILED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::ProcessResponse [%p] FAILED [0x%X]\n%s\n", 
            m_poPrdyContext, prdy_dr, pszResponse )); 
    }
    return dr;
}

IXDRM_HRESULT CXDrm::SendHttp(
        const char* szContentType,
        const char* szRequest,
        char** pszResponse,
        const char* szURL) 
{ 
    IXDRM_HRESULT dr = pkS_OK;

    CXHttp* poHttp = NULL;
    
    std::string httpBody = szRequest;
    std::string httpServer;
    std::string resource = szURL;
    std::string httpRequest;

    CXDRM_LOG_MESSAGE( ("CXDrm::SendHttp URL: %s\n", szURL ) ); 
    
    // NOTE: does NOT _RequireTransaction();

    // Parse out the httpServer and resource strings
    {
        size_t ixProto = resource.find("http://");

        if (0 != ixProto)
        {
            dr = pkE_INVALIDARG;
            goto ErrorExit;
        }


        resource = resource.substr(ixProto+7, resource.size() - (ixProto+7));
        
        size_t ixUri = resource.find_first_of('/');

        if (std::string::npos == ixUri)
        {
            dr = pkE_INVALIDARG;
            goto ErrorExit;
        }

        httpServer = resource.substr(0, ixUri);
        resource = resource.substr(ixUri, resource.size()-(ixUri));
    }
    
    // Create the request header string
    {
        const int uintStrSize = 12; // max string size of %u
        char contentSizeStr[uintStrSize];
        
        StringCbPrintfA(contentSizeStr, uintStrSize, "%u", (unsigned int) httpBody.size());
        
        httpRequest =  "POST " + resource + " HTTP/1.1\r\n";
        httpRequest += 
            "Accept: */*\r\n"
            "Accept-Language: en-US\r\n"
            "Content-Length: "
            ;
        httpRequest += contentSizeStr;
        httpRequest += 
            "\r\n"
            "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n"
            "Pragma: no-cache\r\n"
            ;
        httpRequest += "Host: " + httpServer + "\r\n";
        
        if (NULL != szContentType)
        {
            httpRequest += szContentType;
        }
        else
        {
            httpRequest += "Content-Type: text/xml; charset=utf-8\r\n";
        }

        // finally, a second cr/lf in a row signals end of headers
        httpRequest += "\r\n"; 
    }

    // Perform the HTTP transaction
    {
        char *pchResponseBody = NULL;
        
        poHttp = NEW_NO_THROW CXHttp;
        if(poHttp == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }
        
        std::string responseHeader;

        bool isResponseOk = poHttp->HttpRequestResponse(httpServer, httpRequest, httpBody, responseHeader);

        if (isResponseOk)
        {
            CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::SendHttp challenge:\n%s\n===RESPONSE HEADER===\n%s\n", 
                httpBody.c_str(), responseHeader.c_str()) ); 
        }

        if(!isResponseOk)
        {
            dr = pkE_FAIL;
            goto ErrorExit;
        }


        uint32_t cbContent = poHttp->GetContentLength();

        if( !(cbContent < MAXIMUM_LICACQ_RESPONSE_BODY_SIZE))
        {
            dr = pkE_INSUFFICIENT_BUFFER;
            goto ErrorExit;
        }

        pchResponseBody = (char*)BKNI_Malloc( cbContent + 1 );
        if(pchResponseBody == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }
        

        // Recv response body into buffer
        
        uint32_t ixContent = 0;
        while (ixContent < cbContent)
        {
            int rc = poHttp->Recv((byte*)&pchResponseBody[ixContent], cbContent - ixContent);
            if (rc <= 0)
            {
                CXDRM_LOG_MESSAGE( ("Receiving response body failed rc: %d len: %d received: %d contentLen: %d\n", 
                    rc, 
                    cbContent - ixContent, 
                    ixContent, 
                    poHttp->GetContentLength()));
                
                MemFree(pchResponseBody);
                dr = pkE_FAIL;
                goto ErrorExit;
            }
            ixContent += rc;
        }
        pkASSERT(ixContent == cbContent);

        pchResponseBody[cbContent] = 0; // null terminate the response body

        CXDRM_LOG_MESSAGE_VERBOSE( ("CXDrm::SendHttp response (%u):\n%s\n", cbContent, pchResponseBody ) ); 

        *pszResponse = pchResponseBody;
        // Note: *pszResponse must be freed by caller using IXDrm::MemFree
    }
    
ErrorExit:

    delete poHttp;
    
    if(!pkSUCCEEDED(dr))
    {
        CXDRM_LOG_MESSAGE( ("CXDrm::SendHttp failed: 0x%X\n", dr ) ); 
    }
    return dr;
}

//
// Reset the XDrm instance.
//
// Arguments:   none.
//
// Returns: none.
//
void CXDrm::Reset() 
{ 
    /* do nothing */ 
}

//
// Release a buffer allocated by BKNI_Malloc or other compatible ways.
//
// Arguments:
// [ptr]    Pointer to the buffer being released.
//
// Returns: none.
//
void CXDrm::MemFree( void* ptr )
{
    if(ptr != NULL) BKNI_Free( ptr );
}

//
// Returns the DRM version code.
//
// Arguments:   none.
//
// Returns: XDRM_VERSIONCODE_PLAYREADY.
//
XDRM_VERSION_CODE CXDrm::GetDRMVersionCode() 
{ 
    return XDRM_VERSIONCODE_PLAYREADY;
}

//
// Retrive various properties from the content header.
// Currently only XDRM_PROP_DECRYPTORSETUP is supported.
//
// Arguments:
// [eProperty]      Type of property to be retrieved.
// [pbProperty]     Pointer to a buffer to receive the requested property.
// [pcbProperty]    Pointer to a variable that contains the size of the buffer
//                  during input and receives the actual size of the buffer used
//                  during output.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::GetContentProperty(
    XDRM_CONTENT_PROPERTY eProperty,
    uint8_t* pbProperty,
    uint32_t* pcbProperty )
{
    IXDRM_HRESULT dr = pkE_FAIL;
    DRM_Prdy_Error_e prdy_dr;
    uint32_t cbProperty = 0;
    _RequireTransaction();

    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    switch ( eProperty )
    {
        case XDRM_PROP_DECRYPTORSETUP:
            
            prdy_dr = DRM_Prdy_Get_Buffer_Size(
                    m_poPrdyContext,
                    DRM_Prdy_getBuffer_content_property_decryptor_setup,
                    NULL, 
                    0,
                    &cbProperty,  /* [out] */ 
                    NULL);
            if(prdy_dr != DRM_Prdy_ok)
            {
                dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
                goto ErrorExit;
            }

            if(*pcbProperty < cbProperty)
            {
                dr = pkE_INSUFFICIENT_BUFFER;
                *pcbProperty = cbProperty; /* Return required size */
                goto ErrorExit;
            }
            *pcbProperty = cbProperty;

            prdy_dr =  DRM_Prdy_Content_GetProperty( 
                    m_poPrdyContext,
                    DRM_Prdy_contentGetProperty_eDecryptorSetup,
                    pbProperty,
                    cbProperty);
            if ( prdy_dr == DRM_Prdy_invalid_header || prdy_dr == DRM_Prdy_xml_not_found )
            {
                // Simply means there is no such property.
                dr = pkS_OK;
            }
            else
            {
                dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
                goto ErrorExit;
            }
            break;

        default:
            dr = pkE_FAIL;
    }

ErrorExit:
    return( dr );
}


//
// Extract the property data containing in a DRM protocol response
// (e.g. license response).
//
// Arguments:
// [pbResponse] Pointer to a buffer containing the DRM protocol response.
// [cbResponse] Size of the buffer mentioned above.
// [eProperty]  Type of property data to retrieve.
// [ppszCustomData] Pointer to a pointer of a buffer that receives the property
//              data in the response. The data in the buffer is a NULL terminated
//              string. If there is no property data in the response the pointer to
//              the buffer is NULL.
//
// Returns:
// DRM_Prdy_ok if the function finishes successfully.
// Corresponding error code if error occurrs.
//
IXDRM_HRESULT CXDrm::GetPropertyFromResponse(
    const uint8_t *pbResponse,
    size_t cbResponse,
    XDRM_RESPONSE_PROPERTY eProperty,
    char **ppszPropertyData )
{
    IXDRM_HRESULT dr = pkS_OK;
    DRM_Prdy_Error_e prdy_dr;
    uint8_t *pchData = NULL;
    uint32_t cchData = 0;
    DRM_Prdy_GetBuffer_Type_e dwType = DRM_Prdy_getBuffer_Additional_Response_Data_Custom_Data;

    //
    // This method does not use the wmrm header
    // so _RequireTransaction should not be called
    //
    AutoLock lock(&s_OperationLock);
    
    pkASSERT( m_fInitDRM );

    if (!( pbResponse != NULL && cbResponse > 0 ))
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }


    if( ppszPropertyData -= NULL )
    {
        dr = pkE_INVALIDARG;
        goto ErrorExit;
    }

    *ppszPropertyData = NULL;
    
    switch ( eProperty )
    {
        case XDRM_RESPONSE_CUSTOM_DATA:
            dwType = DRM_Prdy_getBuffer_Additional_Response_Data_Custom_Data;
            break;

        case XDRM_RESPONSE_REDIRECT_URL:
            dwType = DRM_Prdy_getBuffer_Additional_Response_Data_Redirect_Url;
            break;

        default:
            dr = pkE_INVALIDARG;
            goto ErrorExit;
            break;
    }

    prdy_dr = DRM_Prdy_Get_Buffer_Size(m_poPrdyContext,
            dwType,
            pbResponse,
            cbResponse,
            &cchData,
            NULL); 
    if ( prdy_dr == DRM_Prdy_ok )
    {
        pchData = ( uint8_t * )BKNI_Malloc( cchData + 1 );
        if(pchData == NULL)
        {
            dr = pkE_OUTOFMEMORY;
            goto ErrorExit;
        }

        BKNI_Memset( pchData, 0, cchData + 1 );

        prdy_dr = DRM_Prdy_GetAdditionalResponseData(
                m_poPrdyContext,
                (uint8_t *) pbResponse,
                cbResponse,
                dwType,
                (char *)pchData,
                cchData );
        if(prdy_dr != DRM_Prdy_ok)
        {
            dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
            goto ErrorExit;
        }

        pchData[ cchData ] = 0;
    }
    else if ( dr == DRM_Prdy_xml_not_found )
    {
        dr = DRM_Prdy_ok;
    }
    else
    {
        dr = DRM_Prdy_Err_To_XDRM_HRESULT(prdy_dr);
        goto ErrorExit;
    }

    *ppszPropertyData = (char*)pchData;

    pchData = NULL;

ErrorExit:
    BKNI_Free( pchData );
    
    return( dr );
}


void CXDrm::BeginTransaction()
{
    //
    // Wait for any existing transaction to complete before starting another one
    //
    s_OperationLock.Lock();

    // TODO: should there be some kind of failure if this wait goes on too long?
    while( m_fInTransaction )
    {
        s_OperationLock.Unlock();
        Executive_Sleep( DRM_TRANSACTION_LOOP_DELAY );
        s_OperationLock.Lock();
    }
    m_fInTransaction = true;
    
    s_OperationLock.Unlock();
}

void CXDrm::EndTransaction()
{
    s_OperationLock.Lock();
    
    if( !m_fInTransaction )
    {
        pkASSERT( false );
    }
    m_fInTransaction = false;
    
    s_OperationLock.Unlock();
}

void CXDrm::_RequireTransaction()
{
    bool fInTransaction = false;
    s_OperationLock.Lock();
    fInTransaction = m_fInTransaction;
    s_OperationLock.Unlock();
    if( !fInTransaction )
    {
        pkASSERT( false );
    }
}

//
// ========================================================================
// IXDrm factory API implementation
// ========================================================================

IXDRM_HRESULT WINAPI XDRM_CreateInstance( IXDrm **ppXDrm )
{
    return CXDrm::_CreateInstance(ppXDrm);
}

//
// Destroy an IXDrm instance. First the reference count is decremented
// and if is reaches 0, the singleton XDrm instance is destroyed,
// otherwise the singleton XDrm instance is left alone.
//
IXDRM_HRESULT WINAPI XDRM_DestroyInstance( IXDrm *pXDrm )
{
    return CXDrm::_DestroyInstance(pXDrm);
}
