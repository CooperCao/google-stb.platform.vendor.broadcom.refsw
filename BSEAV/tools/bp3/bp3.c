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
#include "cJSON.h"

#include "bchp_common.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_jtag_otp.h"
#include "bchp_bsp_glb_control.h"

#include "bsagelib_types.h"
#include "bkni.h"

#include "nexus_security_types.h"
#include "nexus_security.h"
#include "nexus_base_os.h" // for NEXUS_GetEnv
#include "nexus_platform.h"

#include "sage_app_utils.h"
#include "bp3_session.h"
#include "bchp_scpu_globalram.h"
#include "priv/bsagelib_shared_globalsram.h"
#include "bp3_features.h"

int bp3_host(int, char **);

#define BP3_Error_(x) #x
const char * const bp3_errors[] = {
    BP3_Error_(None),
    BP3_Error_(Unknown),
    BP3_Error_(SessionTokenMismatch),
    BP3_Error_(GetTimeAfterSessionToken),
    BP3_Error_(InvalidSessionTokenTime),
    BP3_Error_(MinTimeSessionTokenNotMet),
    BP3_Error_(FailedToReadBondOption),
    BP3_Error_(InternalDevBondOption),
    BP3_Error_(SageInternal_0),
    BP3_Error_(InvalidIPOwner),
    BP3_Error_(SageInternal_1),
    BP3_Error_(SageInternal_2),
    BP3_Error_(SageInternal_3),
    BP3_Error_(SageInternal_4),
    BP3_Error_(SageInternal_5),
    BP3_Error_(SageInternal_6),
    BP3_Error_(SageInternal_7),
    BP3_Error_(SageInternal_8),
    BP3_Error_(CorruptedConfigBlockDetected),
    BP3_Error_(SageInternal_9),
    BP3_Error_(CorruptedLicenseContractDetected),
    BP3_Error_(SageInternal_10),
    BP3_Error_(SageInternal_11),
    BP3_Error_(InvalidCcfFileVersion),
    BP3_Error_(AntirollbackViolation),
    BP3_Error_(InvalidNumberOfDataBlocks),
    BP3_Error_(InvalidProvisioningType),
    BP3_Error_(ContractPayloadType),
    BP3_Error_(ContractVersion),
    BP3_Error_(ChipInfoPayloadTypeMismatch),
    BP3_Error_(ChipInfoVersionMismatch),
    BP3_Error_(ProductIDMismatch),
    BP3_Error_(ProductSecurityTypeMismatch),
    BP3_Error_(ChipIDMismatch),
    BP3_Error_(DateCodeMismatch),
    BP3_Error_(InvalidIPOwnerID),
    BP3_Error_(SageInternal_12),
    BP3_Error_(MinBP3TAVersionNotMet),
    BP3_Error_(SageInternal_13),
    BP3_Error_(MinSecureVideoTAVersionNotMet),
    BP3_Error_(BinFileLoadedNotBp3BinFile),
    BP3_Error_(AlredayProvisioned),
    BP3_Error_(SageInternal_14),
    BP3_Error_(SageInternal_15),
    BP3_Error_(Bp3BinFileBufferNotPresent),
    BP3_Error_(Bp3LogBufferNotPresent),
    BP3_Error_(Bp3LogBuffSizeInvalid),
    BP3_Error_(SageInternal_16),
    BP3_Error_(SageInternal_17),
    BP3_Error_(SageInternal_18),
    BP3_Error_(SageInternal_19),
    BP3_Error_(SageInternal_20),
    BP3_Error_(CorruptedBp3CcfFileDetected),
    BP3_Error_(SageInternal_21),
    BP3_Error_(SageInternal_22),
    BP3_Error_(BondOptionNotValid),
    BP3_Error_(Internal_1),
    BP3_Error_(ProcessCcfFileHeader),
    BP3_Error_(ProcessSessionToken),
    BP3_Error_(ProcessBp3CcfFile),
    BP3_Error_(SageInternal_23),
    BP3_Error_(SageInternal_24),
    BP3_Error_(SageInternal_25),
    BP3_Error_(ProvBlock),
    BP3_Error_(SageInternal_26),
    BP3_Error_(InFieldProvisioningUpdateFailed),
    BP3_Error_(SageInternal_27),
    BP3_Error_(SageInternal_28),
    BP3_Error_(GenSessionToken),
    BP3_Error_(GetTimeAtSessionTokenGen),
    BP3_Error_(SageInternal_29),
    BP3_Error_(SageInternal_30),
    BP3_Error_(SageInternal_31),
    BP3_Error_(SageInternal_32),
    BP3_Error_(SageInternal_33),
    BP3_Error_(SageInternal_34),
    BP3_Error_(SageInternal_35),
    BP3_Error_(SageInternal_36),
    BP3_Error_(SageInternal_37),
    BP3_Error_(SageInternal_38),
    BP3_Error_(SageInternal_39),
    BP3_Error_(SageInternal_40),
    BP3_Error_(SageInternal_41),
    BP3_Error_(SageInternal_42),
    BP3_Error_(SageInternal_43),
    BP3_Error_(SageInternal_44),
    BP3_Error_(SageInternal_45),
    BP3_Error_(ReadIpDisableReg),
    BP3_Error_(WriteIpDisableReg),
    BP3_Error_(ReadGenCtrl0Reg),
    BP3_Error_(WriteGenCtrl0Reg),
    BP3_Error_(ResetQAM),
    BP3_Error_(DolbyAudioFeatureWithoutBrcm),
    BP3_Error_(DolbyHostFeatureWithoutBrcm),
    BP3_Error_(RoviFeatureWithoutBrcm),
    BP3_Error_(TechnicolorFeatureWithoutBrcm),
    BP3_Error_(DTSFeatureWithoutBrcm),
    BP3_Error_(SageInternal),
    BP3_Error_(SageInternal_46),
    BP3_Error_(ProvisioningDisallowedPart),
    BP3_Error_(SageInternal_47),
    BP3_Error_(SageInternal_48),
    BP3_Error_(InvalidBp3BinFileSize),
    BP3_Error_(CRC32Mismatch),
    BP3_Error_(InsufficientBp3BinFileMem),
    BP3_Error_(LicenseSequenceNumberInCCFNotInBp3Bin),
    BP3_Error_(eSageInternal_49),
    BP3_Error_(eInsufficientMemForFeatureList),
    BP3_Error_(eInvalidFeatureListReadSize),
    BP3_Error_(eFailedToReadProductID),
    BP3_Error_(eFailedToReadBondOptionReg),
    BP3_Error_(eSageInternal_50),
    BP3_Error_(eInvalidLicenseBondOption),
    BP3_Error_(eLicenseBondOptionMismatch)
};

BDBG_MODULE(bp3_curl);

extern char bp3_bin_file_name[];
extern char bp3_bin_file_path[];
extern bp3featuresStruct bp3_features[];

typedef struct inputfile_params_t {
  char user[256];
  char password[256];
  char portal[256];
  char key[256];
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

static long response_code = 0;

static int read_part_number(uint32_t *prodId, uint32_t *securityCode) {
  int rc = NEXUS_Platform_ReadRegister(BCHP_SUN_TOP_CTRL_PRODUCT_ID, prodId);
  if (rc) return rc;
  return NEXUS_Platform_ReadRegister(BCHP_JTAG_OTP_GENERAL_STATUS_8, securityCode);
}

static int read_features(uint32_t *features) {
    int i, rc = 0;
    for(i=0; i< GlobalSram_IPLicensing_Info_size; i++)
    {
      rc = NEXUS_Platform_ReadRegister(BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eIPLicensing) + i*4, features+i);
      if (rc) return rc;
    }
    return rc;
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

#if CURL_AT_LEAST_VERSION(7,56,0)
static void reset(CURL *curl, curl_mime **multipart) {
  curl_mime_free(*multipart);
  *multipart = NULL;
#else
static void reset(CURL *curl, struct curl_httppost **post, struct curl_httppost **last) {
  curl_formfree(*post);
  *post = NULL;
  *last = NULL;
#endif
  curl_easy_reset(curl);
  //show messages for debugging
//  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

  // SSL
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

#if CURL_AT_LEAST_VERSION(7,56,0)
#define ADD_PART(name, fmt, ...) \
    part = curl_mime_addpart(multipart); \
    curl_mime_name(part, name); \
    sprintf(buf, fmt, __VA_ARGS__); \
    curl_mime_data(part, buf, CURL_ZERO_TERMINATED);
#else
#define ADD_PART(name, fmt, ...) \
    sprintf(buf, fmt, __VA_ARGS__); \
    curl_formadd(&post, &last, \
      CURLFORM_COPYNAME, name, \
      CURLFORM_COPYCONTENTS, buf, CURLFORM_END);
#endif

#define CHECK_ERROR(rc, fmt, ...) \
  if (rc) { \
      fprintf(stderr, "Error: " fmt, ##__VA_ARGS__); \
      goto leave; \
   }

#define CHECK_ERROR_RESP(rc, fmt, ...) \
    CHECK_ERROR(rc, fmt, ##__VA_ARGS__) \
    else { \
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code); \
      CHECK_ERROR((response_code == 400 || response_code == 401 || response_code == 404), fmt, ##__VA_ARGS__) \
    }

static bool isNumber(const char* str, const bool commaOK) {
  for (int i = 0, l = strlen(str); i < l; i++)
    if (!isdigit(str[i]) && (!commaOK || str[i] != ','))
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

  printf("%s", prompt);
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
  params->user[0] = '\0';
  params->password[0] = '\0';
  params->key[0] = '\0';
  params->license[0] = '\0';
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
    else if (strstr(buf, "key: ") == buf) {
      const char *key = &buf[5];
      memcpy(params->key, key, strlen(key)+1);
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
  printf("\nusage:\n");
  printf("%s status\n", appName);
  printf("  list features already provisioned on this device\n\n");
  printf("%s service [-A ip_address]\n", appName);
  printf("  Run bp3 in host service mode, aka bp3 host. Optionally specify the ip address if more than one NIC.\n\n");
  printf("%s provision\n", appName);
  printf("  provisions the device using input file bp3.provision.config\n\n");
  printf("%s provision -portal <server url> [ -key <key> ] -license <comma delimited license IDs>\n", appName);
  printf("  where:\n"
         "    -portal: bp3 portal site\n"
         "    -key: your 32 digit API key from your bp3 portal user account\n"
         "    -license: comma delimited license IDs to provision with\n"
         "  Note: key option is available only after License Portal March 2018 release.\n"
         "        If key is omitted, bp3 will prompt you for user/password\n");
  printf("  Example: %s provision -portal https://bp3.broadcom.com -key e906224b7cca497da11c1433c7157d4f -license 3,4\n", appName);
  return -1;
}

static int status(int argc, char *argv[])
{
  uint32_t feats[GlobalSram_IPLicensing_Info_size];
  int rc = read_features(feats);
  CHECK_ERROR(rc, "Unable to read global SRAM for features\n")

  /* prints UId */
  uint32_t otpIdHi = 0, otpIdLo = 0;
  uint8_t *session = NULL;
  uint32_t sessionSize = 0;
  rc = bp3_session_start(&session, &sessionSize);
  CHECK_ERROR(rc, "Unable to start bp3 session\n")
  rc = bp3_get_otp_id(&otpIdHi, &otpIdLo);
  CHECK_ERROR(rc, "Unable to read Chip ID\n")
  printf("UId = 0x%08x%08x\n\n", otpIdHi, otpIdLo);
  if (session)
    bp3_session_end(NULL, 0, NULL, NULL, NULL, NULL);

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
      printf("%s - %s [Enabled]\n", ipOwners[bp3_features[i].OwnerId], bp3_features[i].Name);
    }
    else
    {
      printf("%s - %s [Disabled]\n", ipOwners[bp3_features[i].OwnerId], bp3_features[i].Name);
    }
  }

#if 0 // to be enabled with BP3 TA v4.0.9
  uint32_t prodId = 0;
  uint32_t securityCode = 0;
  uint32_t bp3SageStatus = 0;
  uint8_t  featureList[20]; // ptr to audio, video0, video1, host, sage - starting with byte 0, 0 enabled
  uint32_t featureListSize = 20;
  uint32_t bondOption = 0xFFFFFFFF;
  bool     provisioned = false;

  rc = bp3_ta_start();
  CHECK_ERROR(rc, "Unable to start BP3 TA\n")

  rc = bp3_get_chip_info (
    (uint8_t *)featureList,
    featureListSize,
    &prodId,
    &securityCode,
    &bondOption,
    &provisioned);
  CHECK_ERROR(rc, "Unable to read chip information\n")
/*
  printf("prodId: 0x%08x, securityCode: 0x%08x\n",prodId, securityCode);
  printf("bondOption: 0x%08x\n",bondOption);
  printf("provisioned = %s\n",provisioned==true ? "true" : "false");
  int index;
  for (index=0; index<20; index += 4)
  {
    printf("0 enabled featureList[%d] = 0x%02x%02x%02x%02x\n",
      index,featureList[index+3],featureList[index+2],featureList[index+1],featureList[index]);
  }
*/
  bp3_ta_end();
#endif


leave:
  return rc;
}

enum api_ver {v0 = 0, v1};
#define SET_API_KEY \
    if (headers) { \
      curl_slist_free_all(headers); \
      headers = NULL; \
    } \
    snprintf(buf, sizeof(buf), "Authorization: bearer %s", apitoken); \
    headers = curl_slist_append(headers, buf); \
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

static int provision(int argc, char *argv[])
{
  char buf[2048];
  char apiVersion[] = "v1";
  uint32_t otpIdHi = 0, otpIdLo = 0;
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  CURL *curl = curl_easy_init();
  if (curl == NULL) {
    perror("Error: curl_easy_init failed\n");
    return CURLE_FAILED_INIT;
  }

  struct curl_slist *headers = NULL;
  int rc = 0;
#if CURL_AT_LEAST_VERSION(7,56,0)
  curl_mime *multipart = NULL;
  curl_mimepart *part = NULL;
  reset(curl, &multipart);
#else
  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;
  reset(curl, &post, &last);
#endif

  curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/bp3_cookie.txt"); // set to an non existing file

  char *server = NULL, *license = NULL;
  char *username = NULL, *password = NULL;
  char *apikey = NULL, *apitoken = NULL;
  int long_index = 0;
  static struct option long_options[] = {
    {"portal",  required_argument, 0,  's' },
    {"key", required_argument, 0,  'k' },
    {"license", required_argument, 0,  'l' },
    {0, 0, 0, 0}
  };
  while ((rc = getopt_long_only(argc, argv, "skl",
    long_options, &long_index )) != -1)
    switch (rc) {
    case 's':
      server = optarg;
      break;
    case 'k':
      apikey = optarg;
      break;
    case 'l':
      license = (char*) malloc(256);
      if (!license) {
        perror("Error: malloc(256) for license failed. Out of memory.\n");
        return -1;
      }
      strcpy(license, argv[optind - 1]);
      for (int i = optind; i < argc; i++) {
        if (!isNumber(argv[i], true)) break;
        if (strlen(license) + strlen(argv[i]) > sizeof(license)) {
          license = (char*) realloc(license, sizeof(license) + 256);
          if (!license) {
            perror("Error: realloc extra 256 bytes for license failed. Out of memory.\n");
            return -1;
          }
        }
        strcat(license, argv[i]);
      }
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
    if (params.key[0] != '\0') {
      apikey = strdup(params.key);
    }
  }

  if ((server == NULL) || (license == NULL)) {
    rc = _usage(argv[0]);
    goto leave;
  }

  if (apikey == NULL) {
    getinput(buf, "Please enter user name\n");
    if (buf[0] != '\0')
      username = strdup(&buf[0]);

    getinput(buf, "Please enter password\n");
    if (buf[0] != '\0')
      password = strdup(&buf[0]);

    if ((username == NULL || password == NULL)) {
      rc = _usage(argv[0]);
      goto leave;
    }
  }

  BDBG_LOG(("Server: %s", server));
  enum api_ver apiVer = v1;
  for (; apiVer >= v0; apiVer--) {
  if (username && password) {
    /* get api key and token */
    if (apiVer == v1)
      snprintf(buf, sizeof(buf), "%s/api/%s/Token/Key", server, apiVersion);
    else
      snprintf(buf, sizeof(buf), "%s/api/account/login", server);
    curl_easy_setopt(curl, CURLOPT_URL, buf);
    char *u = curl_easy_escape(curl, username, strlen(username));
    char *p = curl_easy_escape(curl, password, strlen(password));
    snprintf(buf, sizeof(buf), "username=%s&password=%s", u, p);
    curl_free(u);
    curl_free(p);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);

    curl_mem_t apiTokenKey;
    memset(&apiTokenKey, 0, sizeof(curl_mem_t));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &apiTokenKey);
    rc = curl_easy_perform(curl);
    if (apiVer == v1) {
      if (rc) continue;
      CHECK_ERROR_RESP(rc, "Authentication on server %s failed. %s\n", server, apiTokenKey.memory ? (char*) apiTokenKey.memory : rc == CURLE_OK ? "" : curl_easy_strerror(rc))
      cJSON *root = cJSON_Parse(apiTokenKey.memory);
      char *token = cJSON_GetObjectItem(root, "token")->valuestring;
      char *apiKey = cJSON_GetObjectItem(root, "apiKey")->valuestring;
      apikey = strdup(apiKey);
      apitoken = strdup(token);
      cJSON_Delete(root);
    }
#if CURL_AT_LEAST_VERSION(7,56,0)
    reset(curl, &multipart);
#else
    reset(curl, &post, &last);
#endif
    if (apiVer == v0) break;
    if (apikey == NULL) {
      rc = _usage(argv[0]);
      goto leave;
    }
  }

  if (apitoken == NULL) {
    /* get api token using api key */
    snprintf(buf, sizeof(buf), "%s/api/%s/Token", server, apiVersion);
    curl_easy_setopt(curl, CURLOPT_URL, buf);
    snprintf(buf, sizeof(buf), "apiKey=%s", apikey);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);

    curl_mem_t apiTokenKey;
    memset(&apiTokenKey, 0, sizeof(curl_mem_t));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &apiTokenKey);

    rc = curl_easy_perform(curl);
    CHECK_ERROR_RESP(rc, "Authentication token on server %s failed. %s\n", server, apiTokenKey.memory ? (char*) apiTokenKey.memory : rc == CURLE_OK ? "" : curl_easy_strerror(rc))
    cJSON *root = cJSON_Parse(apiTokenKey.memory);
    char *token = cJSON_GetObjectItem(root, "token")->valuestring;
    apitoken = strdup(token);
    cJSON_Delete(root);

#if CURL_AT_LEAST_VERSION(7,56,0)
    reset(curl, &multipart);
#else
    reset(curl, &post, &last);
#endif
  }

  if (apitoken == NULL) {
    rc = _usage(argv[0]);
    goto leave;
  }
  if (apiVer == v1) break;
  }

  // read ids
  uint32_t prodId, securityCode;
  rc = read_part_number(&prodId, &securityCode);
  CHECK_ERROR(rc, "Unable to read chip id and security code\n")

  uint8_t *session = NULL;
  uint32_t sessionSize = 0;
  rc = bp3_session_start(&session, &sessionSize);
  CHECK_ERROR(rc, "Unable to start bp3 session\n")

  rc = bp3_get_otp_id(&otpIdHi, &otpIdLo);
  CHECK_ERROR(rc, "Unable to read Chip ID\n")
  if (otpIdHi == 0 && otpIdLo == 0) {
    NEXUS_Platform_ReadRegister(BCHP_BSP_GLB_CONTROL_v_PubOtpUniqueID_hi, &otpIdHi);
    NEXUS_Platform_ReadRegister(BCHP_BSP_GLB_CONTROL_v_PubOtpUniqueID_lo, &otpIdLo);
  }
  printf("UId = 0x%08x%08x\n", otpIdHi, otpIdLo);

#if 0 // to be enabled with BP3 TA v4.0.9
    uint32_t bp3SageStatus;
    uint8_t  featureList[20]; // ptr to audio, video0, video1, host, sage
    uint32_t featureListSize = 20;
    uint32_t bondOption;
    bool     provisioned;

    rc = bp3_get_chip_info (
      (uint8_t *)featureList,
      featureListSize,
      &prodId,
      &securityCode,
      &bondOption,
      &provisioned);
    CHECK_ERROR(rc, "Unable to read chip information\n")
    /*
    printf("new prodId: 0x%08x, securityCode: 0x%08x\n",prodId, securityCode);
    printf("bondOption: 0x%08x\n",bondOption);
    printf("provisioned = %s\n",provisioned ? "true" : "false");
    int index;
    for (index=0; index<20; index += 4)
    {
      printf("0 enabled featureList[%d] = 0x%02x%02x%02x%02x\n",
         index,featureList[index+3],featureList[index+2],featureList[index+1],featureList[index]);
    }
    */
#endif

  /* get ccf */
  if (apiVer == v0)
    snprintf(buf, sizeof(buf), "%s/api/provision/ccf", server);
  else {
    SET_API_KEY
    snprintf(buf, sizeof(buf), "%s/api/%s/provision/ccf", server, apiVersion);
  }
  curl_easy_setopt(curl, CURLOPT_URL, buf);
#if CURL_AT_LEAST_VERSION(7,56,0)
  multipart = curl_mime_init(curl);
#endif
  ADD_PART("otpId", "0x%x%08x", otpIdHi, otpIdLo)
  ADD_PART("otpSelect", "%zu", BP3_OTPKeyTypeA)
  ADD_PART("prodId", "0x%x", prodId)
  ADD_PART("secCode", "0x%x", securityCode & 0x03FFC000)

  char *token = strtok(license, ",");
  while (token != NULL) {
    CHECK_ERROR(!isNumber(token, false), "Invalid license ID: %s . License ID should be comma separated.\n", token)
    BDBG_LOG(("License ID: %s", token));
    ADD_PART("lic", "%s", token)
    token = strtok(NULL, ",");
  }
  free(license);

  if (apiVer == v0) {
#if CURL_AT_LEAST_VERSION(7,56,0)
  part = curl_mime_addpart(multipart);
  curl_mime_name(part, "token");
  curl_mem_t tok;
  tok.memory = session;
  tok.size = sessionSize;
  tok.position = 0;
  curl_mime_filedata(part, "token.bin");
  curl_mime_data_cb(part, sessionSize, read_mem_callback, NULL, NULL, &tok);

  curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);
#else
  curl_formadd(&post, &last,
    CURLFORM_COPYNAME, "token",
    CURLFORM_BUFFER, "token.bin",
    CURLFORM_BUFFERPTR, session,
    CURLFORM_BUFFERLENGTH, (long) sessionSize,
    CURLFORM_END);
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
#endif
  }
  else
  /* token */
  for (int i=0; i<sessionSize; i++)
  {
    ADD_PART("token", "%d", session[i])
  }

#if CURL_AT_LEAST_VERSION(7,56,0)
  curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);
#else
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
#endif

  curl_mem_t ccf;
  memset(&ccf, 0, sizeof(curl_mem_t));
  /* send all data to this function  */
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
  /* pass 'ccf' struct to the callback function */
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &ccf);

  rc = curl_easy_perform(curl);
  CHECK_ERROR_RESP(rc, "create CCF failed. %s\n", ccf.memory ? (char*) ccf.memory : rc == CURLE_OK ? "" : curl_easy_strerror(rc))
#if CURL_AT_LEAST_VERSION(7,56,0)
    reset(curl, &multipart);
#else
    reset(curl, &post, &last);
#endif

  /* provision */
  uint8_t *logbuf = NULL;
  uint32_t logsize = 0;
  uint32_t* status = NULL; /* status by IP owner */
  uint32_t statusSize = 0;
  int errCode = bp3_session_end(ccf.memory, ccf.size, &logbuf, &logsize, &status, &statusSize);
  session = NULL; // bp3_session_end freed bp3_session_end
  if (ccf.memory) free(ccf.memory);
  if (errCode != 0)
    fprintf(stderr, "Provision failed with error: %s\n", errCode < 0 ? "Unexpected" : errCode < sizeof(bp3_errors)/sizeof(bp3_errors[0]) ? bp3_errors[errCode] : "Other new error");

  // upload log and bp3.bin
  if (apiVer == v0)
    snprintf(buf, sizeof(buf), "%s/api/provision/result", server);
  else {
    SET_API_KEY
    /* upload logFile and bp3File */
    snprintf(buf, sizeof(buf), "%s/api/%s/provision/uploadlog", server, apiVersion);
  }
  curl_easy_setopt(curl, CURLOPT_URL, buf);

#if CURL_AT_LEAST_VERSION(7,56,0)
  multipart = curl_mime_init(curl);
#endif
  for (uint32_t i = 0; i < statusSize; i++) {
    ADD_PART("status", "%zu", status[i])
  }
  free(status);

  ADD_PART("errCode", "%zu", errCode)

  /* logFile */
  if (logsize > 0) {
#if CURL_AT_LEAST_VERSION(7,56,0)
    part = curl_mime_addpart(multipart);
    curl_mime_name(part, apiVer == v0 ? "log" : "logFile");
    curl_mem_t log;
    log.memory = logbuf;
    log.size = logsize;
    log.position = 0;
    curl_mime_filedata(part, "log.bin");
    curl_mime_data_cb(part, logsize, read_mem_callback, NULL, NULL, &log);
#else
    curl_formadd(&post, &last,
      CURLFORM_COPYNAME, apiVer == v0 ? "log" : "logFile",
      CURLFORM_BUFFER, "log.bin",
      CURLFORM_BUFFERPTR, logbuf,
      CURLFORM_BUFFERLENGTH, (long) logsize,
      CURLFORM_END);
#endif
  }

  /* bp3File */
  if (errCode == 0) {
    snprintf(buf, sizeof(buf), "%s/%s", bp3_bin_file_path, bp3_bin_file_name);
#if CURL_AT_LEAST_VERSION(7,56,0)
    part = curl_mime_addpart(multipart);
    curl_mime_name(part, apiVer == v0 ? "bp3" : "bp3File");
    curl_mime_filedata(part, buf);
#else
    curl_formadd(&post, &last,
      CURLFORM_COPYNAME, apiVer == v0 ? "bp3" : "bp3File",
      CURLFORM_FILE, buf,
      CURLFORM_END);
#endif
  }

#if CURL_AT_LEAST_VERSION(7,56,0)
  curl_easy_setopt(curl, CURLOPT_MIMEPOST, multipart);
#else
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
#endif

  rc = curl_easy_perform(curl);
  if (logbuf) free(logbuf);
  CHECK_ERROR_RESP(rc, "Upload log failed. %s\n", rc == CURLE_OK ? "" : curl_easy_strerror(rc))

leave:
  if ((response_code == 400) || (response_code == 401) || (response_code == 404))
    rc = 1;

  if (session)  // bp3_session_end never called! Call it to free memory
    bp3_session_end(NULL, 0, NULL, NULL, NULL, NULL);
#if CURL_AT_LEAST_VERSION(7,56,0)
  if (multipart) curl_mime_free(multipart);
#else
  if (post) curl_formfree(post);
#endif
  curl_slist_free_all(headers);
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
  else if (strcmp(argv[1], "service") == 0)
    rc = bp3_host(argc, argv);
  else
    _usage(argv[0]);

leave:
  /* Leave Nexus: Finalize platform ... */
  SAGE_app_leave_nexus();
leave_nexus:
  if (rc)
    fprintf(stderr, "%s ERROR #%d\n", __FUNCTION__, rc);
  return rc;
}
