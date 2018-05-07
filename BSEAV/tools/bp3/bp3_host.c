/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_jtag_otp.h"
#include "bchp_bsp_glb_control.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <byteswap.h>
#include "nexus_platform.h"

#include "civetweb.h"
#include "bp3_session.h"
#include "bchp_scpu_globalram.h"
#include "priv/bsagelib_shared_globalsram.h"

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

uint32_t chipProdID = 0;
uint32_t securityCode = 0;
BP3_Otp_KeyType otpSelect = BP3_OTPKeyTypeA;
uint32_t otpIdHi = 0;
uint32_t otpIdLo = 0;
extern char ip_addr[INET_ADDRSTRLEN];

uint8_t *ccfbuf = NULL;
uint32_t ccfsize = 0;
uint8_t *logbuf = NULL;
uint32_t logsize = 0;
uint32_t *_status = NULL; /* status by IP owner */
uint32_t statusSize = 0;
int errorCode = 0;
pthread_t ssdpThread = 0;

extern char bp3_bin_file_name[];
extern char bp3_bin_file_path[];

// TODO: Partners should define this port
int port = 80;
int quit = 0;

/* SIGINT handler: set quit to 1 for graceful termination */
static void
handle_sigint(int signum UNUSED_PARAM) {
  quit = 1;
}

void run_ssdp(int port, const char *pFriendlyName, const char * pModelName, const char *pUuid);
void get_local_address(void);
int request_handler(struct mg_connection *, void *);

static void mg_write_header(struct mg_connection *conn, const char* mime) {
  mg_printf(conn, "HTTP/1.1 200 OK\r\n"
  "Content-Type: %s\r\n"
  "Connection: keep-alive\r\n"
  "\r\n", mime);
}

static int web_chip_info_handler(struct mg_connection *conn, void *cbdata)
{
  mg_write_header(conn, "text/plain");
  mg_printf(conn, "0x%X,0x%X,0x%X%08X", chipProdID, securityCode, otpIdHi, otpIdLo);
  return 200;
}

static int web_otp_id_handler(struct mg_connection *conn, void *cbdata)
{
  mg_write_header(conn, "text/plain");
  mg_printf(conn, "%d,0x%X%08X", otpSelect, otpIdHi, otpIdLo);
  return 200;
}

static int web_post_bp3_handler(struct mg_connection *conn, void *cbdata)
{
  const struct mg_request_info *req = mg_get_request_info(conn);
  ccfsize = req->content_length;
  if (ccfbuf) free(ccfbuf);
  ccfbuf = (uint8_t*) malloc(sizeof(uint8_t) * ccfsize);
  if (ccfbuf == NULL) {
    mg_send_http_error(conn, 500, "Unable to allocate %d bytes of memory", ccfsize);
    errorCode = -1;
    return 500;
  }
  if (_status) {
    free(_status);
    _status = NULL;
  }
  for (int i = mg_read(conn, ccfbuf, ccfsize), l = i; i > 0 && l < ccfsize; l += i > 0 ? i : 0)
    i = mg_read(conn, ccfbuf + l, ccfsize - l);
  errorCode = bp3_session_end(ccfbuf, ccfsize, &logbuf, &logsize, &_status, &statusSize);
  free(ccfbuf);
  ccfbuf = NULL;
  ccfsize = 0;
  if (errorCode == 0) {
    mg_write_header(conn, "text/plain");
    return 200;
  }
  else {
    mg_send_http_error(conn, 500, "Provision failed with error %d", errorCode);
    return 500;
  }
}

static int web_get_log_handler(struct mg_connection *conn, void *cbdata)
{
  if (logbuf) {
    mg_write_header(conn, "application/octet-stream");
    mg_write(conn, logbuf, logsize);
    free(logbuf);
    logbuf = NULL;
    logsize = 0;
    return 200;
  }
  else {
    mg_send_http_error(conn, 500, "%s", "There is no log");
    return 500;
  }
}

static int web_get_token_handler(struct mg_connection *conn, void *cbdata)
{
  uint8_t *session = NULL;
  uint32_t size;
  if (bp3_session_start(&session, &size)) {
    mg_send_http_error(conn, 500, "%s", "Unable to start bp3 session");
    return 500;
  }
  mg_write_header(conn, "application/octet-stream");
  mg_write(conn, session, size);
  return 200;
}

static int web_get_bp3_handler(struct mg_connection *conn, void *cbdata)
{
  unsigned char buf[256];
  snprintf(buf, sizeof(buf), "%s/%s", bp3_bin_file_path, bp3_bin_file_name);
  mg_send_mime_file(conn, buf, "application/octet-stream");
  return 200;
}

static int web_get_status_handler(struct mg_connection *conn, void *cbdata)
{
  mg_write_header(conn, "text/plain");
  for (size_t i = 0; i < statusSize; i++)
    mg_printf(conn, "%d,", _status[i]);
  mg_printf(conn, "%d", errorCode);
  return 200;
}

static int
read_features(uint32_t *features) {
  int i, rc = 0;
  for(i=0; i< GlobalSram_IPLicensing_Info_size; i++)
  {
    rc = NEXUS_Platform_ReadRegister(BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eIPLicensing) + i*4, features+i);
    if (rc) return rc;
  }
  return rc;
}

static int web_get_features_handler(struct mg_connection *conn, void *cbdata)
{
  uint32_t v[GlobalSram_IPLicensing_Info_size];
  read_features(v);

  mg_write_header(conn, "text/plain");
  for (size_t i = 0; i < GlobalSram_IPLicensing_Info_size; i++)
    mg_printf(conn, "0x%x,", v[i]);
  return 200;
}

// copied from libuuid
typedef unsigned char uuid_t[16];
struct uuid {
  uint32_t  time_low;
  uint16_t  time_mid;
  uint16_t  time_hi_and_version;
  uint16_t  clock_seq;
  uint8_t node[6];
};

void uuid_unpack(const uuid_t in, struct uuid *uu)
{
  const uint8_t *ptr = in;
  uint32_t    tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  tmp = (tmp << 8) | *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_low = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_mid = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_hi_and_version = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->clock_seq = tmp;

  memcpy(uu->node, ptr, 6);
}
static void uuid_unparse_x(const uuid_t uu, char *out, const char *fmt)
{
  struct uuid uuid;

  uuid_unpack(uu, &uuid);
  sprintf(out, fmt,
    uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
    uuid.clock_seq >> 8, uuid.clock_seq & 0xFF,
    uuid.node[0], uuid.node[1], uuid.node[2],
    uuid.node[3], uuid.node[4], uuid.node[5]);
}
static const char *fmt_lower =
    "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";
void uuid_unparse(const uuid_t uu, char *out)
{
  uuid_unparse_x(uu, out, fmt_lower);
}
// end of libuuid

static struct mg_context *start_webserver()
{
    char port_str[11];
    snprintf(port_str, sizeof(port_str), "%d", port);

    const char *options[] = {
        "listening_ports",
        port_str,
        0};
    mg_init_library(MG_FEATURES_DEFAULT);
    struct mg_context *ctx = mg_start(NULL, NULL, options);

    mg_set_request_handler(ctx, "/dd.xml", request_handler, 0);
    mg_set_request_handler(ctx, "/Chip", web_chip_info_handler, 0);
    mg_set_request_handler(ctx, "/Id", web_otp_id_handler, 0);
    mg_set_request_handler(ctx, "/Token", web_get_token_handler, 0);
    mg_set_request_handler(ctx, "/File", web_post_bp3_handler, 0);
    mg_set_request_handler(ctx, "/Log", web_get_log_handler, 0);
    mg_set_request_handler(ctx, "/Bin", web_get_bp3_handler, 0);
    mg_set_request_handler(ctx, "/Status", web_get_status_handler, 0);
    mg_set_request_handler(ctx, "/Features", web_get_features_handler, 0);
    return ctx;
}

static void *ssdpProc(void *args UNUSED_PARAM) {
  unsigned char buf[21], model[21];
  buf[snprintf((char *) buf, sizeof(buf), "BP3 %X", chipProdID)] = '\0';
  model[snprintf((char *) model, sizeof(buf), "BP3 %X", securityCode)] = '\0';
  uuid_t uu;
  uint32_t x;
  x = bswap_32(securityCode >> 12);
  memcpy(uu, &x, 4);
  x = bswap_32(chipProdID & 0xFFFFF000); // assume 12 revision bits
  memcpy(uu + 4, &x, 4);
  x = bswap_32(otpIdHi);
  memcpy(uu + 8, &x, 4);
  x = bswap_32(otpIdLo);
  memcpy(uu + 12, &x, 4);
  char usn[37];
  uuid_unparse(uu, usn);
  strncpy(usn, "bp30", 4); // last digit is version number
  struct mg_context *ctx = start_webserver();
  run_ssdp(port, buf, model, usn);
  mg_stop(ctx);
  mg_exit_library();
  printf("\nWeb server exited\n");
  return NULL;
}

static int run( int argc, char **argv  )
{
  int result;
  char addr_str[NI_MAXHOST] = "::";
  int opt;

  while ((opt = getopt(argc, argv, "AP:")) != -1) {
    switch (opt) {
    case 'A':
      strncpy(addr_str, optarg, NI_MAXHOST - 1);
      addr_str[NI_MAXHOST - 1] = '\0';
      break;
    case 'P':
      port = atoi(optarg);
      break;
    }
  }

  if (strcmp(addr_str, "::") == 0)
    get_local_address();
  else
    strcpy(ip_addr, addr_str);

  result = pthread_create(&ssdpThread, NULL, ssdpProc, &quit);
  if (result != 0) {
    printf("Can't create thread :[%s]", strerror(result));
    return result;
  }
  return 0;
}

int start_bp3_host( int argc, char **argv ) {
  int rc = NEXUS_Platform_ReadRegister(BCHP_SUN_TOP_CTRL_PRODUCT_ID, &chipProdID);
  if (rc) {
    perror("BCHP_SUN_TOP_CTRL_PRODUCT_ID");
    return rc;
  }
  rc = NEXUS_Platform_ReadRegister(BCHP_JTAG_OTP_GENERAL_STATUS_8, &securityCode);
  if (rc) {
    perror("BCHP_JTAG_OTP_GENERAL_STATUS_8");
    return rc;
  }

  uint8_t *session = NULL;
  uint32_t sessionSize = 0;
  rc = bp3_session_start(&session, &sessionSize);
  if (rc) {
    perror("Unable to start bp3 session");
    return rc;
  }

  rc = bp3_get_otp_id(&otpIdHi, &otpIdLo);
  if (rc) {
    perror("bp3_get_otp_id");
    return rc;
  }
  if (otpIdHi == 0 && otpIdLo == 0) {
    NEXUS_Platform_ReadRegister(BCHP_BSP_GLB_CONTROL_v_PubOtpUniqueID_hi, &otpIdHi);
    NEXUS_Platform_ReadRegister(BCHP_BSP_GLB_CONTROL_v_PubOtpUniqueID_lo, &otpIdLo);
  }
  if (session)
    bp3_session_end(NULL, 0, NULL, NULL, NULL, NULL);

  return run(argc, argv);
}

int bp3_host( int argc, char **argv )
{
  signal(SIGINT, handle_sigint);
  int rc = start_bp3_host(argc, argv);
  if (ssdpThread)
    pthread_join(ssdpThread, NULL);
  return rc;
}

void stop_bp3_host() {
  quit = 1;
  if (ssdpThread)
    pthread_join(ssdpThread, NULL);
}
