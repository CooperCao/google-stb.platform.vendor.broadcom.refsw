/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "v3d_debug.h"

#include <stdio.h>

#include "interface/khronos/include/EGL/egl.h"
#include "vcfw/drivers/chip/abstract_v3d.h"

#define V3D_DBG_TIMEOUT  100 // simple wait-busy time out loop

#define V3D_DBSCS_IDLE     0x0001 // idle flag
#define V3D_DBSCS_SHIFTSR  0x0002 // shift client address (16 bits)
#define V3D_DBSCS_SHIFTDR  0x0004 // shift user data (1-128 bits)
#define V3D_DBSCS_LOAD     0x0008 // make load pulse
#define V3D_DBSCS_STORE    0x0010 // make store pulse

// Device flags.
#define V3D_DEVFL_READ    0x0001   // device can be read
#define V3D_DEVFL_WRITE   0x0002   // device can be written
#define V3D_DEVFL_LOAD    0x0004   // device needs 'load' pulse
#define V3D_DEVFL_STORE   0x0008   // device needs 'store' pulse
// These are not yet used or implemented in the code:
#define V3D_DEVFL_START6  0x0010   // set flag 6 before access
#define V3D_DEVFL_START7  0x0020   // set flag 7 before access
#define V3D_DEVFL_FINISH6 0x0040   // set flag 6 after access
#define V3D_DEVFL_FINISH7 0x0080   // set flag 7 after access
#define V3D_DEVFL_VOLAT   0x0100   // device is volatile: requires restore

typedef struct {
  char *name;              // name of device
  int  bits;               // 'scan' length
  int  flags;              // Status & control of device
  int  dev_address;        // address of device
  int  dev_low,dev_high;   // 'memory' address range
} client_device_type;


typedef struct {
  char *name;      // Name of client
  int  devices;    // number of 'devices'
  client_device_type device[32]; // this has to be at the end
} debug_client_type;

static debug_client_type vpm  = {
  "vpm",
  4,   // num devices

  { {
  "id map",
  49,                 // 49 bits long
                      // first is 'valid' bit then 48 block-bits
  V3D_DEVFL_READ,     // device can be read
  0,                  // Dev. address
  0,15                // min & max address (inclusive)
  },
  {
  "mem read",
#ifdef __BCM2708A0__
  512,
#else
  256,
#endif
  V3D_DEVFL_READ |
  V3D_DEVFL_LOAD,     // device can be read & needs load pulse
  1,
#ifdef __BCM2708A0__
  0,255
#else
  0,511
#endif
  },
  {
  "mem web",
#ifdef __BCM2708A0__
  64,
#else
  32,
#endif
  V3D_DEVFL_READ | V3D_DEVFL_WRITE,// device can be read written & shift is destructive
  2,
  0,0
  },
  {
  "mem write",
#ifdef __BCM2708A0__
  512,
#else
  256,
#endif
  V3D_DEVFL_READ | V3D_DEVFL_WRITE | V3D_DEVFL_STORE,// device can be read written & shift is destructive
  3,
#ifdef __BCM2708A0__
  0,255
#else
  0,511
#endif
  } }

  };

static debug_client_type qpu  = {
  "qpu",
  27, // num devices
  { {
  "R0.0",
  128,                 // 128 bits long
  V3D_DEVFL_READ | V3D_DEVFL_WRITE, // readable & writable
  0,                  // Dev. address
  0,0                 // min & max address (inclusive)
  },
  {"R1.0",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  1, 0,   0},
  {"R2.0",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  2, 0,   0},
  {"R3.0",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  3, 0,   0},
  {"R0.1",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  4, 0,   0},
  {"R1.1",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  5, 0,   0},
  {"R2.1",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  6, 0,   0},
  {"R3.1",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  7, 0,   0},
  {"R0.2",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  8, 0,   0},
  {"R1.2",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE,  9, 0,   0},
  {"R2.2",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 10, 0,   0},
  {"R3.2",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 11, 0,   0},
  {"R0.3",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 12, 0,   0},
  {"R1.3",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 13, 0,   0},
  {"R2.3",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 14, 0,   0},
  {"R3.3",  128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 15, 0,   0},
  {"flags",  48, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 16, 0,   0}, // flags_c,flags_n,flags_z
  {"R4",    128, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 17, 0,   3},
  {"R5",     32, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 18, 0,   3},
  {"pc0",    29, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 19, 0,   0},
  {"pc1",    29, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 20, 0,   0},
  {"RA",    128, V3D_DEVFL_READ | V3D_DEVFL_WRITE
                  | V3D_DEVFL_LOAD | V3D_DEVFL_STORE, 21, 0, 127},
  {"RB",    128, V3D_DEVFL_READ | V3D_DEVFL_WRITE
                  | V3D_DEVFL_LOAD | V3D_DEVFL_STORE, 22, 0, 127},
  {"qr.0",  122, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 23, 0,   1}, // qr_msm[64],qr_qy[20],qr_qx[20],qr_rev[4],qr_txy[14]
  {"qr.1",   37, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 24, 0,   1}, // qr_ucb[30],qr_zcfg[5],qr_hdrmode,qr_covmode
  {"qr.2",   72, V3D_DEVFL_READ | V3D_DEVFL_WRITE, 25, 0,   0}, // qr_stcfg_b[36], qr_stcfg_f[36]
#ifdef __BCM2708A0__
  {"thrd",   24, V3D_DEVFL_READ                  , 26, 0,   0}
  // thread,t0_done,t1_done,t0_ttype,t1_ttype,t_user,t0_qvalid[4],t1_qvalid[4],
  // t_hattr[4],t_hvert[4],t0_rflip,t1_rflip
  #define THRD_RFLIP_SHIFT 22
#else
  {"thrd",   25, V3D_DEVFL_READ                  , 26, 0,   0}
  // thread,t0_done,t1_done,t0_ttype[2],t1_ttype[2],t0_qvalid[4],t1_qvalid[4],
  // t_hattr[4],t_hvert[4],t0_rflip,t1_rflip
  #define THRD_RFLIP_SHIFT 23
#endif
   }
};


static int num_client = 13;

static debug_client_type *debug_client[] = {&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&qpu,&vpm,};
static char *debug_client_name[] = {"QPU 0","QPU 1","QPU 2","qpu3","QPU 4","QPU 5","QPU 6","qpu7",
                             "QPU 8","QPU 9","QPU 10","qpu11","VPM"};

#ifdef V3D_DEBUG_TEST

// todo : return error status
static void debug_test_qpu(int cl);
static void debug_test_vpm(int cl);

// enough space to work on a reg file
static int data[512];
static int store[512];

void v3d_debug_test(void)
{ int cl,dev,*ip;
  v3d_write(DBCFG, 0x01); // enable debugging

  // only testing QPUs for now
  for (cl=0; cl<num_client; cl++)
  { if ( (debug_client[cl]->name[0]=='q') &&
         (debug_client[cl]->name[1]=='p') )
       debug_test_qpu(cl);
     else
       debug_test_vpm(cl);
  }

  return;
}

/*
 * Check two buffers against each other
 * buffers have a number of units each a number of bits wide
 */
static void check_store(int bits,int units,int *ref,int *read,char *name)
{ int mask;
  int b;
  int ref_word,read_word;

  while (units--)
  { b = bits;
    while (b)
    { ref_word = *ref++;
      read_word = *read++;
      if (b>=32)
      { mask = 0xFFFFFFFF;
        b -=32;
      }
      else
      {
        mask = 0xFFFFFFFF >> (32-b);
        b=0;
      }
      ref_word  &= mask;
      read_word &= mask;
      if (ref_word!=read_word)
      {
         printf("%s", name);
         printf("%s", " expected 0x");
         printf("%08X", ref_word);
         printf("%s", " read 0x");
         printf("%08X", read_word);
         printf("%s", "\n");
      }
      else
         printf("%s", ".");

    } // check bits
  } // check units
  printf("%s", name);
  printf("%s", "\n");
} // check_store

/*
 * Test debug chains of a QPU
 * This tests only the loading and reading from
 * the debug registers throught the debug module.
 *
 * return 0 if OK,
 * other value on error.
 */
static void debug_test_qpu(int cl)
{ int ac,r,*ip,f;
  int ad;
  printf("%s", (debug_client[cl]->name));
  printf("%s", "\n======\n");

  //
  // Checking does NOT use bit fields from the debug struct
  // as an extra measure to see if our bits fields have the right size
  //


  // fill data with a pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = (f & 0x0FF) | ((~f<<8) & 0x00FF00) | 0x12340000;
  // clear storage
  ip = store;
  for (f=0; f<512; f++)
    ip[f] = 0;

  // load each accumulator
  for (ac=0; ac<16; ac++)
     v3d_debug_write_device(cl,ac,0,1,data+ac);
  // unload each accumulator (128 bits wide)
  for (ac=0; ac<16; ac++)
  {
     v3d_debug_read_device(cl,ac,0,1,store);
     check_store(128,1,data+ac,store,"AC");
  }

  // load flags
  v3d_debug_write_device(cl,16,0,1,data+16);
  // unload flags 12 per QPU
  v3d_debug_read_device(cl,16,0,1,store);
  check_store(4*12,1,data+16,store,"FL");

  // acc r4 & 5
  v3d_debug_write_device(cl,17,0,4,data);
  v3d_debug_write_device(cl,18,0,4,data+16);

  v3d_debug_read_device(cl,17,0,4,store);
  check_store(128,1,data,store,"AC4");
  v3d_debug_read_device(cl,18,0,4,store);
  check_store( 32,1,data+16,store,"AC5");

  // pc0 & pc1
  v3d_debug_write_device(cl,19,0,1,data+32);
  v3d_debug_write_device(cl,20,0,1,data+64);

  v3d_debug_read_device(cl,19,0,1,store);
  check_store(29,1,data+32,store,"PC0");
  v3d_debug_read_device(cl,20,0,1,store);
  check_store(29,1,data+64,store,"PC1");


  // reg file A
  // clear storage
  ip = store;
  for (f=0; f<512; f++)
    ip[f] = 0;
  for (ad=0; ad<128; ad++)
    v3d_debug_write_device(cl,21,ad,1,data+ad);
  for (ad=0; ad<128; ad++)
  {
     v3d_debug_read_device(cl,21,ad,1,store);
     check_store(128,1,data+ad,store,"RA");
     // check
  }

  // reg file B
  // clear storage
  ip = store;
  for (f=0; f<512; f++)
    ip[f] = 0;
  for (ad=0; ad<128; ad++)
    v3d_debug_write_device(cl,22,ad,1,data+ad);
  for (ad=0; ad<128; ad++)
  {
     v3d_debug_read_device(cl,22,ad,1,store);
     check_store(128,1,data+ad,store,"RB");
  }

  // difficult to verify qr... in waveform :-(
  // qr set 0 & 1
  ip = store;
  for (f=0; f<512; f++)
    ip[f] = 0;
  v3d_debug_write_device(cl,23,0,2,data);
  v3d_debug_write_device(cl,24,0,2,data+16);
  v3d_debug_read_device(cl,23,0,2,store);
  check_store(122,2,data,store,"QR0");
  v3d_debug_read_device(cl,24,0,2,store);
  check_store( 37,2,data+16,store,"QR1");

  // qr set 2
  v3d_debug_write_device(cl,25,0,1,data+32);
  v3d_debug_read_device(cl,25,0,1,store);
  check_store(72,1,data+32,store,"QR2");

  // thread data
  v3d_debug_read_device(cl,26,0,1,store);

} // debug_test_qpu


/*
 * Read/write VPM test
 *
 * reminder:
 *  chain 0 = malloc
 *  chain 1 = mem read
 *  chain 2 = mem web
 *  chain 3 = mem write
 */
static void debug_test_vpm(int cl)
{ int a,*ip,f;

  // NO read/write test for malloc chain as that is read-only!

  // read/write VPM
  // set all 64 write enables to 1
  data[0]=0xFFFFFFFF; data[1]=0xFFFFFFFF;
  v3d_debug_write_device(cl,2,0,1,data);

  // fill data with a pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = (f & 0x0FF) | ((~f<<8) & 0x00FF00) | 0x12340000;

  // write pattern into memory
  v3d_debug_write_device(cl,3,0,32,data);

  // Invert pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = ~ip[f];
  // write again
  v3d_debug_write_device(cl,3,32,32,data);

  // 'checkerboard' pattern
  ip = data;
  for (f=0; f<512; f++)
     ip[f] = 0xAAAA^ip[f];
  // write again
  v3d_debug_write_device(cl,3,64,32,data);

  // Invert pattern
  ip = data;
  for (f=0; f<512; f++)
     ip[f] = ~ip[f];
  // write again
  v3d_debug_write_device(cl,3,96,32,data);

  // 'checkerboard' pattern
  ip = data;
  for (f=0; f<512; f++)
     ip[f] = 0x5555^ip[f];
  // write again
  v3d_debug_write_device(cl,3,128,32,data);

  // Invert pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = ~ip[f];
  v3d_debug_write_device(cl,3,160,32,data);

  //
  // Complete VPM memory should be full now!!
  //

  // fill data with a pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = (f & 0x0FF) | ((~f<<8) & 0x00FF00) | 0x12340000;
  // clear storage
  ip = store;
  for (f=0; f<512; f++)
    ip[f] = 0;

  // read the memory
  v3d_debug_read_device(cl,1,0,32,store);
  check_store(512,32,data,store,"VPM 0-31");

  // Invert pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = ~ip[f];

  // read the memory again
  v3d_debug_read_device(cl,1,32,32,store);
  check_store(512,32,data,store,"VPM 32-64");

  // 'checkerboard' pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = 0xAAAA^ip[f];
  v3d_debug_read_device(cl,1,64,32,store);
  check_store(512,32,data,store,"VPM 64-95");

  // Invert pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = ~ip[f];
  v3d_debug_read_device(cl,1,96,32,store);
  check_store(512,32,data,store,"VPM 96-127");

  // 'checkerboard' pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = 0x5555^ip[f];
  v3d_debug_read_device(cl,1,128,32,store);
  check_store(512,32,data,store,"VPM 128-159");

  // Invert pattern
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = ~ip[f];
  v3d_debug_read_device(cl,1,160,32,store);
  check_store(512,32,data,store,"VPM 160-191");

  // Test write enables
  data[0]=0x55555555; data[1]=0xAAAAAAAA;
  v3d_debug_write_device(cl,2,0,1,data);
  ip = data;
  for (f=0; f<512; f++)
    ip[f] = (f & 0x0FF) | ((~f<<8) & 0x00FF00) | 0x12340000;
  // write pattern into memory using special web pattern
  v3d_debug_write_device(cl,3,0,32,data);

  // clear storage
  ip = store;
  for (f=0; f<512; f++)
    ip[f] = 0;
  // read the memory
  v3d_debug_read_device(cl,1,0,32,store);

  // Mix store & data to what we expect
  for (f=0; f<16; f++)
    store[f] = (store[f] & 0x00FF00FF) | (data[f] & 0xFF00FF00);
  for (f=16; f<32; f++)
    store[f] = (store[f] & 0xFF00FF00) | (data[f] & 0x00FF00FF);
  check_store(512,32,data,store,"VPM WEB");

} // debug_test_vpm

#endif

/*
 * Wait for idle bit to be set
 * return 1 if done
 * return 0 on timeout
 *
 */
static int wait_debug_ready()
{ int wait;
  wait = V3D_DBG_TIMEOUT;
  do {
    if (v3d_read(DBSCS) & V3D_DBSCS_IDLE)
      return 1;
  } while (wait--);
  return 0; // false
}

/*
 *
 * Read data from V3D debug chain
 *
 * All data is read as 32 bits wide and stored as such.
 * Unused data bits are NOT masked off
 */
int v3d_debug_read_device(int client,int dev,int address,int units,unsigned int *store)
{ int d,dev_adrs,bits;
  client_device_type *client_pntr;
//  if (!enabled)
//    return V3D_DBG_ERR_NOEN; // debug not enabled
  if (client<0 || client>=num_client)
    return V3D_DBG_ERR_NOCL; // No such client
  d = debug_client[client]->devices;
  if (dev<0 || dev>=d)
    return V3D_DBG_ERR_NODV; // No such device
  client_pntr = &(debug_client[client]->device[dev]);
  if (units<0 || address<client_pntr->dev_low || address+units-1>client_pntr->dev_high)
    return V3D_DBG_ERR_ILAD; // illegal address
  if (!(client_pntr->flags & V3D_DEVFL_READ))
    return V3D_DBG_ERR_ILOP; // illegal operation

  // All parameters are correct
  // (except store which we have to assume to be correct)
  // Now do one or more reads

  v3d_write(DBSCFG, client); // select client

  while (units--)
  { // select device & location inside device
    dev_adrs  = client_pntr->dev_address;
    dev_adrs |= ((address & 0xff) << 8) | ((address & 0x100) >> 2);
    v3d_write(DBSSR, dev_adrs);          // set address
    v3d_write(DBSCS, V3D_DBSCS_SHIFTSR); // shift address out
    if (!wait_debug_ready()) {
      v3d_write(DBSCFG, 0x3f);
      return V3D_DBG_ERR_TIME; // timeout
    }

    if (client_pntr->flags & V3D_DEVFL_LOAD)
      v3d_write(DBSCS, V3D_DBSCS_LOAD); // make load pulse

    // read all bits from device
    bits = client_pntr->bits;
    while (bits)
    { d = bits>128 ? 128 : bits;
      v3d_write(DBSCS, (d<<8) | V3D_DBSCS_SHIFTDR); // shift 'd' client data bits
      if (!wait_debug_ready()) {
        v3d_write(DBSCFG, 0x3f);
        return V3D_DBG_ERR_TIME; // timeout
      }

      *store++ = v3d_read(DBSDR0);
      if (bits>32)
         *store++ = v3d_read(DBSDR1);
      if (bits>64)
         *store++ = v3d_read(DBSDR2);
      if (bits>96)
         *store++ = v3d_read(DBSDR3);

      bits -= d;
    } // while bits
    address = address + 1;
  } // while units

  v3d_write(DBSCFG, 0x3f);

  return 0;
} // v3d_debug_read_device



/*
 *
 */
int v3d_debug_write_device(int client,int dev,int address,int units,unsigned int *data)
{
  int d,dev_adrs,bits;
  client_device_type *client_pntr;
//  if (!enabled)
//    return V3D_DBG_ERR_NOEN; // debug not enabled
  if (client<0 || client>=num_client)
    return V3D_DBG_ERR_NOCL; // No such client
  d = debug_client[client]->devices;
  if (dev<0 || dev>=d)
    return V3D_DBG_ERR_NODV; // No such device
  client_pntr = &(debug_client[client]->device[dev]);
  if (units<0 || address<client_pntr->dev_low || address+units-1>client_pntr->dev_high)
    return V3D_DBG_ERR_ILAD; // illegal address
  if (!(client_pntr->flags & V3D_DEVFL_WRITE))
    return V3D_DBG_ERR_ILOP; // illegal operation

  // All parameters are correct
  // (except data which we have to assume to be correct)
  // Now do one or more writes

  v3d_write(DBSCFG, client); // select client

  while (units--)
  { // select device & location inside device
    dev_adrs  = client_pntr->dev_address;
    dev_adrs |= ((address & 0xff) << 8) | ((address & 0x100) >> 2);
    v3d_write(DBSSR, dev_adrs);          // set address
    v3d_write(DBSCS, V3D_DBSCS_SHIFTSR); // shift address out
    if (!wait_debug_ready()) {
      v3d_write(DBSCFG, 0x3f);
      return V3D_DBG_ERR_TIME; // timeout
    }


    // write all bits to device
    bits = client_pntr->bits;
    while (bits)
    {

      v3d_write(DBSDR0, *data++);
      if (bits>32)
         v3d_write(DBSDR1, *data++);
      if (bits>64)
         v3d_write(DBSDR2, *data++);
      if (bits>96)
         v3d_write(DBSDR3, *data++);

      d = bits>128 ? 128 : bits;
      v3d_write(DBSCS, (d<<8) | V3D_DBSCS_SHIFTDR); // shift 'd' client data bits
      if (!wait_debug_ready()) {
        v3d_write(DBSCFG, 0x3f);
        return V3D_DBG_ERR_TIME; // timeout
      }

      bits -= d;
    } // while bits
    if (client_pntr->flags & V3D_DEVFL_STORE)
      v3d_write(DBSCS, V3D_DBSCS_STORE); // make store pulse
    address = address + 1;
  } // while units

  v3d_write(DBSCFG, 0x3f);

  return 0;
} // v3d_debug_write_device

//
// TODO:
// read & write device at the same time.
// Required to store incoming data from 'volatile' chains
// so they can be restored afterwards
//

unsigned int v3d_debug_read_pc(int qpu, int thread)
{
   unsigned int pc;
   v3d_debug_read_device(qpu, 19 + thread, 0, 1, &pc);
   v3d_debug_write_device(qpu, 19 + thread, 0, 1, &pc);
   return pc << 3;
}

unsigned int v3d_debug_read_unif(int qpu, int thread)
{
   unsigned int unif[2];
   v3d_debug_read_device(qpu, 24, thread, 1, unif);
   v3d_debug_write_device(qpu, 24, thread, 1, unif);
   return ((unif[1] << 25) | (unif[0] >> 7)) << 2;
}

void v3d_debug_read_r(int qpu, int i, unsigned int *r)
{
   if (i < 4) {
      int j;
      for (j = 0; j != 4; ++j) {
         v3d_debug_read_device(qpu, (j * 4) + i, 0, 1, r + (j * 4));
         v3d_debug_write_device(qpu, (j * 4) + i, 0, 1, r + (j * 4));
      }
   } else if (i == 4) {
      v3d_debug_read_device(qpu, 17, 0, 4, r);
      v3d_debug_write_device(qpu, 17, 0, 4, r);
   } else if (i == 5) {
      v3d_debug_read_device(qpu, 18, 0, 4, r);
      v3d_debug_write_device(qpu, 18, 0, 4, r);
      int j;
      for (j = 15; j != 0; --j) {
         r[j] = r[j >> 2];
      }
   } /* else: not permitted */
}

static unsigned int read_thrd(int qpu, int thread)
{
   unsigned int thrd;
   v3d_debug_read_device(qpu, 26, 0, 1, &thrd);
   /* read is non-destructive, no need to write back */
   return thrd;
}

void v3d_debug_read_ra(int qpu, int thread, int i, unsigned int *ra)
{
   i ^= thread << 4;
   /* if rflip is set for this thread, the lowest bit of the address is flipped by the qpus. we should do the same */
   i ^= (read_thrd(qpu, thread) >> (THRD_RFLIP_SHIFT + thread)) & 1;

   if (i >= 16) { i += 48; }
   int j;
   for (j = 0; j != 4; ++j) {
      v3d_debug_read_device(qpu, 21, (j * 16) + i, 1, ra + (j * 4));
   }
}

void v3d_debug_read_rb(int qpu, int thread, int i, unsigned int *rb)
{
   i ^= thread << 4;
   /* if rflip is set for this thread, the lowest bit of the address is flipped by the qpus. we should do the same */
   i ^= (read_thrd(qpu, thread) >> (THRD_RFLIP_SHIFT + thread)) & 1;

   if (i >= 16) { i += 48; }
   int j;
   for (j = 0; j != 4; ++j) {
      v3d_debug_read_device(qpu, 22, (j * 16) + i, 1, rb + (j * 4));
   }
}

void v3d_debug_read_flags(int qpu, unsigned char *flags)
{
   unsigned int f[2];
   v3d_debug_read_device(qpu, 16, 0, 1, f);
   v3d_debug_write_device(qpu, 16, 0, 1, f);
   int i;
   for (i = 0; i != 16; ++i) {
      flags[i] =
         ((f[0] & (1 << i)) ? 2 : 0) |        /* z */
         ((f[0] & (1 << (16 + i))) ? 1 : 0) | /* n */
         ((f[1] & (1 << i)) ? 4 : 0);         /* c */
   }
}

void v3d_debug_write_pc(int qpu, int thread, unsigned int pc)
{
   pc >>= 3;
   v3d_debug_write_device(qpu, 19 + thread, 0, 1, &pc);
}

void v3d_debug_read_vpm_row(int i, unsigned int *row)
{
   unsigned int r[16];
   int j;
#ifdef __BCM2708A0__
   v3d_debug_read_device(12, 1, i, 1, r);
   for (j = 0; j != 16; ++j) {
      row[j] = r[((i + j) & 0xf)
         /* v3d_debug_read_device writes out the 128-bit chunks in the wrong order */
         ^ 0xc];
   }
#else
   v3d_debug_read_device(12, 1, ((i >> 4) << 5) | (i & 0xf), 1, r);
   v3d_debug_read_device(12, 1, ((i >> 4) << 5) | (i & 0xf) | 0x10, 1, r + 8);
   for (j = 0; j != 16; ++j) {
      row[j] = r[(((i + j) & 0x7)
         /* v3d_debug_read_device writes out the 128-bit chunks in the wrong order */
         ^ 0x4) | (j & 0x8)];
   }
#endif
}
