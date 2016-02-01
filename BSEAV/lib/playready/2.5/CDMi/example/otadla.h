/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef __OTADLA_H__
#define __OTADLA_H__

#include <stdint.h>

// Acquire a license from a license server specified by a URL
// using the supplied license challenge and get back a license
// response.
bool GetLicense(
    __in_z char *f_pszURL,
    __in_bcount(f_cbChallenge) uint8_t *f_pbChallenge,
    __in uint32_t f_cbChallenge,
    __deref_out_bcount(*f_pcbResponse) uint8_t **f_ppbResponse,
    __out uint32_t *f_pcbResponse);

#endif  // __OTADLA_H__
