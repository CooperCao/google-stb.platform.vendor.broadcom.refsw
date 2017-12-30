#include "copyrt.h"
/* remove the following define if you aren't running WIN32 */
/* #define WININC 0 */

#ifdef WININC
#include <windows.h>
#else
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#endif

#include "global.h"
/* change to point to where MD5 .h's live; RFC 1321 has sample
   implementation */
#include "md5.h"

/* set the following to the number of 100ns ticks of the actual
   resolution of your system's clock */
#define UUIDS_PER_TICK 1024

/* Set the following to a calls to get and release a global lock */
#define LOCK
#define UNLOCK

/* Set this to what your compiler uses for 64-bit data type */
#ifdef WININC
typedef unsigned long   unsigned32;
typedef unsigned short  unsigned16;
typedef unsigned char   unsigned8;
typedef unsigned char   byte;
#define unsigned64 unsigned __int64
#define I64(C) C
#else
typedef uint64_t   unsigned64;
typedef uint32_t   unsigned32;
typedef uint16_t   unsigned16;
typedef uint8_t    unsigned8;
typedef uint8_t    byte;
#define I64(C) C##LL
#endif

typedef unsigned64 uuid_time_t;
typedef struct {
    char nodeID[6];
} uuid_node_t;

void get_ieee_node_identifier(uuid_node_t *node);
void get_system_time(uuid_time_t *uuid_time);
void get_random_info(char seed[16]);
