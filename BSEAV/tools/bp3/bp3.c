/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <curl/curl.h>
#include <ctype.h>
#include <getopt.h>

#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_jtag_otp.h"
#include "bchp_bsp_glb_control.h"

#include "bsagelib_types.h"
#include "bkni.h"

#include "nexus_security_types.h"
#include "nexus_security.h"
#include "nexus_base_os.h" // for NEXUS_GetEnv

#include "sage_app_utils.h"
#include "bp3_session.h"
#include "bchp_scpu_globalram.h"
#include "priv/bsagelib_shared_globalsram.h"
#include "bp3_features.h"

BDBG_MODULE(bp3_curl);

extern char bp3_bin_file_name[];
extern char bp3_bin_file_path[];
extern bp3featuresStruct bp3_features[];

typedef struct inputfile_params_t {
  char user[256];
  char password[256];
  char portal[256];
  char license[256];
} inputfile_params;

static bitmapStruct bitmap[16 * 8];
static const char *ipOwners[] = {"Unused", "Broadcom", "Dolby", "Rovi", "Technicolor", "DTS"};
static uint32_t features[GlobalSram_IPLicensing_Info_size];

static struct {
    uint8_t OwnerId;
    eSramMap SramMap[4];
} owners[5] = {
    {1, {Video0, Video1, Host, Audio0} },
    {2, {Audio0, Host, Reserved, NotUsed} },
    {3, {Host, Reserved, ReservedLast, NotUsed} },
    {4, {Reserved, Host, ReservedLast, NotUsed} },
    {5, {Audio0, Reserved, ReservedLast, NotUsed} }
};

static int read_part_number(uint32_t *chipFamilyID, uint32_t *securityCode) {
  MAP_MEM_START

  MAP_START(BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID, 1)
  *chipFamilyID = *(volatile uint32_t*) (page + addr);
  MAP_END

  MAP_START(BCHP_JTAG_OTP_GENERAL_STATUS_8, 1)
  *securityCode = *(volatile uint32_t*) (page + addr) & 0x03FFC000;
  MAP_END

  return MAP_MEM_END
}

static int read_features(uint32_t *features) {
  MAP_MEM_START
  MAP_START(BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eIPLicensing), GlobalSram_IPLicensing_Info_size)
  memcpy(features, page + addr, GlobalSram_IPLicensing_Info_size * sizeof(uint32_t));
  MAP_END
  return MAP_MEM_END
}

typedef struct _curl_memory_t {
  uint8_t *memory;
  curl_off_t size;
  curl_off_t position;
} curl_mem_t;

static size_t write_mem_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  curl_mem_t *mem = (curl_mem_t *) userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    perror("not enough memory (realloc returned NULL)");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

static size_t read_mem_callback(char *buffer, size_t size, size_t nitems, void *arg)
{
  curl_mem_t *p = (curl_mem_t *) arg;
  curl_off_t realsize = p->size - p->position;

  nitems *= size;
  if(realsize > nitems)
    realsize = nitems;
  if(realsize)
    memcpy(buffer, p->memory + p->position, realsize);
  p->position += realsize;
  return realsize;
}


static void reset(CURL *curl, curl_mime **multipart) {
  curl_mime_free(*multipart);
  *multipart = NULL;
  curl_easy_reset(curl);
  //show messages for debugging
//  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  // SSL
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

#define ADD_PART(name, fmt, ...) \
    part = curl_mime_addpart(multipart); \
    curl_mime_name(part, name); \
    sprintf(buf, fmt, __VA_ARGS__); \
    curl_mime_data(part, buf, CURL_ZERO_TERMINATED);

#define CHECK_ERROR(rc, fmt, ...) \
  if (rc) { \
      fprintf(stderr, "Error: " fmt, ##__VA_ARGS__); \
      goto leave; \
   }

#define CHECK_ERROR_RESP(rc, fmt, ...) \
    CHECK_ERROR(rc, fmt, ##__VA_ARGS__) \
    else { \
      long response_code; \
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code); \
      CHECK_ERROR((response_code == 400), fmt, ##__VA_ARGS__) \
    }

static bool isNumber(const char* str) {
  for (int i = 0, l = strlen(str); i < l; i++)
    if (!isdigit(str[i]))
      return false;
  return true;
}

static bool isOn(uint8_t owner, uint8_t bit)
{
    bitmapStruct i = bitmap[bit];
    return ((features[(int)owners[owner-1].SramMap[i.key]] & i.value) == 0);
}

static int getch() {
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

static void getinput(char *input, char *prompt)
{
  int i = 0;
  char c, asterisk = '*';
  struct termios t_old, t_new;

  printf(prompt);
  tcgetattr(STDIN_FILENO, &t_old);
  t_new = t_old;
  t_new.c_lflag &= ~(ICANON | ECHO);
  t_new.c_cc[VMIN] = 1;
  t_new.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &t_new);

  while (read (STDIN_FILENO, &c, 1) && (isalnum(c) || ispunct(c))
     && i < 256 - 2)
  {
    input[i++] = c;
    write (STDOUT_FILENO, &asterisk, 1);
  }

  input[i] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
  printf("\n");
}

static int parse_provision_file(const char *filename, inputfile_params *params)
{
  FILE *f = fopen(filename, "r");
  if (!f) {
    BDBG_MSG(("unable to open %s", filename));
    return -1;
  }
  while (!feof(f)) {
    char buf[256];
    unsigned len;
    fgets(buf, sizeof(buf), f);
    len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[--len] = 0;
    if (len && buf[len-1] == '\r') buf[--len] = 0;
    if (strstr(buf, "portal: ") == buf) {
      const char *portal = &buf[8];
      memcpy(params->portal, portal, strlen(portal)+1);
    }
    else if (strstr(buf, "user: ") == buf) {
      const char *user = &buf[6];
      memcpy(params->user, user, strlen(user)+1);
    }
    else if (strstr(buf, "password: ") == buf) {
      const char *password = &buf[10];
      memcpy(params->password, password, strlen(password)+1);
    }
    else if (strstr(buf, "license: ") == buf) {
      const char *license = &buf[9];
      memcpy(params->license, license, strlen(license)+1);
    }
  }
  fclose(f);
  return 0;
}

static int _usage(const char *appName)
{
  printf("usage:\n");
  printf("%s status\n", appName);
  printf("  list features already provisioned on this device\n");
  printf("  Example: %s status\n\n", appName);
  printf("%s provision\n", appName);
  printf("  provisions the device using input file bp3.provision.config\n\n");
  printf("%s provision -portal -user -password -license <comma delimited license IDs>\n", appName);
  printf("  where:\n"
      "    -portal: bp3 portal site\n"
      "    -user: username to bp3 portal\n"
      "    -password: password to bp3 portal\n"
      "    -license: license IDs to provision\n"
      "  Note: please escape characters such as ! using \\.\n"
      "        if -user or -password is not specifed, it will prompt you to enter username/password\n");
  printf("  Example: %s provision -portal http://bbs-test-rack1.broadcom.com:1800 -user arris -password Arris\\!bp3 -license 3,4\n", appName);
  return -1;
}

static int status(int argc, char *argv[])
{
  uint32_t feats[GlobalSram_IPLicensing_Info_size];
  int rc = read_features(feats);
  CHECK_ERROR(rc, "Unable to read global SRAM for features\n")

  for (int i = 0; i < 16 * 8; i++) {
    bitmap[i].key = i / 32;
    bitmap[i].value = 1 << (i % 32);
  }
  for (int i = 0; i < GlobalSram_IPLicensing_Info_size; i++) {
    features[i] = feats[i];
  }
  /* it appears that host and sage uses only 16 bit, and the higher 16 bit is from sage */
  features[(uint32_t)Host] = (feats[(uint32_t)Host] & 0x0000FFFF) | (feats[(uint32_t)Sage] << 16);

  for (int i = 0; i < BP3_FEATURES_NUM; i++) {
    if (isOn(bp3_features[i].OwnerId, bp3_features[i].Bit))
    {
      printf("%s - %s\n", ipOwners[bp3_features[i].OwnerId], bp3_features[i].Name);
    }
  }
leave:
  return rc;
}

static int provision(int argc, char *argv[])
{
  char buf[256];
  curl_global_init(CURL_GLOBAL_ALL);
  uint32_t otpIdHi = 0, otpIdLo = 0;

  /* get a curl handle */
  CURL *curl = curl_easy_init();
  if (curl == NULL) {
    perror("Error: curl_easy_init failed\n");
    return CURLE_FAILED_INIT;
  }

  int rc = 0;
  curl_mime *multipart = NULL;
  curl_mimepart *part = NULL;
  reset(curl, &multipart);
  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/bp3_cookie.txt"); // set to an non existing file

  char *server = NULL, *license = NULL;
  char *username = NULL, *password = NULL;
  int long_index = 0;
  static struct option long_options[] = {
    {"portal",  required_argument, 0,  's' },
    {"user",    required_argument, 0,  'u' },
    {"password",required_argument, 0,  'p' },
    {"license", required_argument, 0,  'l' },
    {0, 0, 0, 0}
  };
  while ((rc = getopt_long_only(argc, argv, "supl",
    long_options, &long_index )) != -1)
    switch (rc) {
    case 's':
      server = optarg;
      break;
    case 'u':
      username = optarg;
      break;
    case 'p':
      password = optarg;
      break;
    case 'l':
      license = argv[optind-1];
      break;
    default:
      return _usage(argv[0]);
    }

  if (argc == 2) {
    inputfile_params params;
    parse_provision_file("bp3.provision.config", &params);
    if (params.portal[0] != '\0') {
      server = strdup(params.portal);
    }
    if (params.user[0] != '\0') {
      username = strdup(params.user);
    }
    if (params.password[0] != '\0') {
      password = strdup(params.password);
    }
    if (params.license[0] != '\0') {
      license = strdup(params.license);
    }
  }

  if ((server == NULL) || (license == NULL)) {
    rc = _usage(argv[0]);
    goto leave;
  }

  if (username == NULL) {
    getinput(buf, "Please enter user name\n");
    if (buf[0] != '\0') {
        username = strdup(&buf[0]);
    }
  }

  if (password == NULL) {
    getinput(buf, "Please enter password\n");
    if (buf[0] != '\0') {
        password = strdup(&buf[0]);
    }
  }

  if ((username == NULL) || (password == NULL)) {
    rc = _usage(argv[0]);
    goto leave;
  }

  BDBG_LOG(("Server: %s", server));

  snprintf(buf, sizeof(buf), "%s/api/account/login", server, username, password);
  curl_easy_setopt(curl, CURLOPT_URL, buf);
  snprintf(buf, sizeof(buf), "username=%s&password=%s", username, password);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
  rc = curl_easy_perform(curl);
  CHECK_ERROR_RESP(rc, "Login to %s failed. %s\n", server, rc == CURLE_OK ? "" : curl_easy_strerror(rc))
  reset(curl, &multipart);

  // read ids
#if (NEXUS_SECURITY_API_VERSION == 1)
  NEXUS_OtpKeyType otpSelect = find_otp_select();
  rc = read_otp_id(NEXUS_OtpKeyType_eA, &otpIdHi, &otpIdLo);
  CHECK_ERROR(rc, "Unable to read OTP ID A\n")
#else
  BP3_Otp_KeyType otpSelect = find_otp_select();
  unsigned keyIndex = 0; /* 0 for OTP ID A */
  rc = read_otp_id(keyIndex, &otpIdHi, &otpIdLo);
  CHECK_ERROR(rc, "Unable to read OTP ID A\n")
#endif

  uint32_t chipFamilyID, securityCode;
  rc = read_part_number(&chipFamilyID, &securityCode);
  CHECK_ERROR(rc, "Unable to read chip family id and security code\n")

  uint8_t *session = NULL;
  uint32_t sessionSize = 0;
  rc = bp3_session_start(&session, &sessionSize);
  CHECK_ERROR(rc, "Unable to start bp3 session\n")

  /* get ccf */
  snprintf(buf, sizeof(buf), "%s/api/provision/ccf", server);
  curl_easy_setopt(curl, CURLOPT_URL, buf);
  multipart = curl_mime_init(curl);
  ADD_PART("otpId", "0x%x%08x", otpIdHi, otpIdLo)
  ADD_PART("otpSelect", "%zu", otpSelect)
  ADD_PART("prodId", "0x%x", chipFamilyID)
  ADD_PART("secCode", "0x%x", securityCode & 0x03FFC000)

  char *token = strtok(license, ",");
  while (token != NULL) {
    CHECK_ERROR(!isNumber(token), "Invalid license ID: %s . License ID should be comma separated.\n", token)
    BDBG_LOG(("License ID: %s", token));
    ADD_PART("lic", "%s", token)
    token = strtok(NULL, ",");
  }

  part = curl_mime_addpart(multipart);
  curl_mime_name(part, "token");
  curl_mem_t tok;
  tok.memory = session;
  tok.size = sessionSize;
  tok.position = 0;
  curl_mime_filedata(part, "token.bin");
  curl_mime_data_cb(part, sessionSize, read_mem_callback, NULL, NULL, &tok);

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);

  curl_mem_t ccf;
  memset(&ccf, 0, sizeof(curl_mem_t));
  /* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
  /* we pass our 'chunk' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &ccf);

  rc = curl_easy_perform(curl);
  CHECK_ERROR_RESP(rc, "create CCF failed. %s\n", ccf.memory ? (char*) ccf.memory : rc == CURLE_OK ? "" : curl_easy_strerror(rc))
  reset(curl, &multipart);

  // provision
  uint8_t *logbuf = NULL;
  uint32_t logsize = 0;
  uint32_t* status = NULL; /* status by IP owner */
  uint32_t statusSize = 0;
  int errCode = bp3_session_end(ccf.memory, ccf.size, &logbuf, &logsize, &status, &statusSize);
  session = NULL; // bp3_session_end freed bp3_session_end
  if (ccf.memory) free(ccf.memory);

  // upload log and bp3.bin
  snprintf(buf, sizeof(buf), "%s/api/provision/result", server);
  curl_easy_setopt(curl, CURLOPT_URL, buf);

  multipart = curl_mime_init(curl);
  for (int i = 0; i < statusSize; i++) {
    ADD_PART("status", "%zu", status[i])
  }
  free(status);

  ADD_PART("errCode", "%zu", errCode)

  if (logsize > 0) {
    part = curl_mime_addpart(multipart);
    curl_mime_name(part, "log");
    curl_mem_t log;
    log.memory = logbuf;
    log.size = logsize;
    log.position = 0;
    curl_mime_filedata(part, "log.bin");
    curl_mime_data_cb(part, logsize, read_mem_callback, NULL, NULL, &log);
  }

  if (errCode == 0) {
    part = curl_mime_addpart(multipart);
    curl_mime_name(part, "bp3");
    snprintf(buf, sizeof(buf), "%s/%s", bp3_bin_file_path, bp3_bin_file_name);
    curl_mime_filedata(part, buf);
  }
  else
    fprintf(stderr, "Provision failed with error %d\n", errCode);

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);

  rc = curl_easy_perform(curl);
  if (logbuf) free(logbuf);
  CHECK_ERROR_RESP(rc, "Upload log failed. %s\n", rc == CURLE_OK ? "" : curl_easy_strerror(rc))

leave:
  if (session)  // bp3_session_end not called!
    bp3_session_end(NULL, 0, NULL, NULL, NULL, NULL);
  if (multipart) curl_mime_free(multipart);
  if (curl) curl_easy_cleanup(curl);
  curl_global_cleanup();
  if (rc)
    fprintf(stderr, "%s ERROR #%d\n", __FUNCTION__, rc);
  return rc;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    return _usage(argv[0]);

  int rc = 0;

  /* Join Nexus: Initialize platform ... */
  rc = SAGE_app_join_nexus();
  if (rc) goto leave_nexus;

  if (strcmp(argv[1], "provision") == 0)
    rc = provision(argc, argv);
  else if (strcmp(argv[1], "status") == 0)
    rc = status(argc, argv);
  else {
    SAGE_app_leave_nexus();
    return _usage(argv[0]);
  }

leave:
  /* Leave Nexus: Finalize platform ... */
  SAGE_app_leave_nexus();
leave_nexus:
  if (rc)
    fprintf(stderr, "%s ERROR #%d\n", __FUNCTION__, rc);
  return rc;
}
