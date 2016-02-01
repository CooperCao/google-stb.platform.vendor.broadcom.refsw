/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_DEBUG_H
#define V3D_DEBUG_H

// V3D Debug routines error codes
#define V3D_DBG_ERR_NONE 0 // No errors
#define V3D_DBG_ERR_NOEN 1 // debug not enabled
#define V3D_DBG_ERR_NOCL 2 // No such client
#define V3D_DBG_ERR_NODV 3 // No such device
#define V3D_DBG_ERR_ILAD 4 // Illegal address
#define V3D_DBG_ERR_ILOP 5 // Illegal operation
#define V3D_DBG_ERR_TIME 6 // Timeout waiting for ready

#ifdef __cplusplus
extern "C" {
#endif

extern int v3d_debug_read_device(int client, int dev, int address, int units, unsigned int *store);
extern int v3d_debug_write_device(int client, int dev, int address, int units, unsigned int *data);

#define V3D_DEBUG_TEST
#ifdef V3D_DEBUG_TEST
   extern void v3d_debug_test(void);
#endif

/* helpers (built ontop of read_device/write_device) */
extern unsigned int v3d_debug_read_pc(int qpu, int thread);
extern unsigned int v3d_debug_read_unif(int qpu, int thread);
extern void v3d_debug_read_r(int qpu, int i, unsigned int *r);
extern void v3d_debug_read_ra(int qpu, int thread, int i, unsigned int *ra);
extern void v3d_debug_read_rb(int qpu, int thread, int i, unsigned int *rb);
extern void v3d_debug_read_flags(int qpu, unsigned char *flags);
extern void v3d_debug_write_pc(int qpu, int thread, unsigned int pc);
extern void v3d_debug_read_vpm_row(int i, unsigned int *row);

#ifdef __cplusplus
}
#endif

#endif
