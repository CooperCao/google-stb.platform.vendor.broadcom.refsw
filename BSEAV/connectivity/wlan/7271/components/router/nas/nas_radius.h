/*
 * Radius support for Network Access Server
 * $Copyright (C) 2003 Broadcom Corporation$
 * $Id: nas_radius.h 241182 2011-02-17 21:50:03Z gmo $
 */
#if !defined(_NAS_RADIUS_H_)
#define _NAS_RADIUS_H_

#ifdef NAS_RADIUS
extern void radius_forward(nas_t *nas, nas_sta_t *sta, eap_header_t *eap);
extern void radius_dispatch(nas_t *nas, radius_header_t *response);

#define RADIUS_FORWARD(nas, sta, eap)	radius_forward(nas, sta, eap)
#define RADIUS_DISPATCH(nas, radius)	radius_dispatch(nas, radius)

#else

#define RADIUS_FORWARD(nas, sta, eap)
#define RADIUS_DISPATCH(nas, radius)
#endif /* NAS_RADIUS */

#endif /* !defined(_NAS_RADIUS_H_) */
