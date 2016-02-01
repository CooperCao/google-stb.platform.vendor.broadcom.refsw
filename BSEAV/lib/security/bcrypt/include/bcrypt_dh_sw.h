/********************************************************************************************
 * ECC implementaion is from "Freeme" open source, see bellow for license term.
 * 
 *A license????  For anonymously published software?  Yes!  The purpose
 *here is to outline how I would like the software used.  Putting
 *together all this has been a lot of work, and I hope people can
 *respect the purpose behind the software, as spelled out here.
 *
 *	1.  The purpose of this software is to re-assert your rights over
 *    fair use of audio files that you have legally purchased or
 *    otherwise obtained legally.  Please use it for that purpose only.
 *    Do not use it to unprotect files you don't have a legal right to,
 *    or to unprotect legal files for the purpose of re-distributing
 *    them to others who do not have a legal right to the content.  In
 *    other words, in use of this software obey traditional copyright
 *    laws -- but the DMCA may be completely ignored as far as this
 *    license concerned (although you must accept responsibility for
 *    ignoring this law, since it is enforceable).
 *
 *  2. This is free software, without any warranties, guarantees, or any
 *    assurance that it will work as described.  It relies on certain
 *    other software (from Microsoft) operating as it currently does,
 *    so I don't take any responsibility for what happens if Microsoft
 *    updates their software to render this useless, or even if they
 *    put bombs in their new software to erase all your files if they
 *    detect this software.  But I sure hope they wouldn't do that.
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
 *****************************************************************************/

#if !defined( _DH_H )
#define _DH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "bcrypt_datatypes.h"

    typedef struct BCRYPT_DH_st {
        void * DHhandle;         /* stores openssl DH handle */
        uint8_t * pubKey;        /* DH public key in raw data */
        uint8_t * sharedSecret;  /* DH shared secret in raw data */
        uint16_t pubKeyLen;      /* length of the public key, in bytes */
        uint16_t sharedSecretLen;/* length of the shared secret, in bytes */
    } BCRYPT_DH_t;

    BCRYPT_STATUS_eCode BCrypt_DH_FromPem(const uint8_t * pem,
                                          int pemLen,
                                          BCRYPT_DH_t ** pContext);
    BCRYPT_STATUS_eCode BCrypt_DH_Free(BCRYPT_DH_t * context);
    BCRYPT_STATUS_eCode BCrypt_DH_ComputeSharedSecret(BCRYPT_DH_t * context,
                                                      const uint8_t * remotePublicKey,
                                                      int keyLen);

#ifdef __cplusplus
}
#endif

#endif
