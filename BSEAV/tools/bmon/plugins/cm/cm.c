/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
/* NOTES: set_value  /root/snmpset -v2c -cpub 192.168.100.1 1.3.6.1.4.1.4413.2.2.2.1.2.5.2.0 i 1
 *                   ./cm "mib_name=iso.3.6.1.4.1.4413.2.2.2.1.2.5.2.0&mib_type=i&mib_value=1"
 *        set_val    http://10.24.5.186:8888/cm/mib_name=iso.3.6.1.4.1.4413.2.2.2.1.2.5.2.0&mib_type=i&mib_value=1
 *        convert    http://10.24.5.186:8888/cm/mibs=iso.3.6.1.4.1.4413.2.2.2.1.2.5.1.1.2.100000000
 *                   ./cm "mibs=iso.3.6.1.4.1.4413.2.2.2.1.2.5.1.1.2.100000000"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <assert.h>

#include <ctype.h>
#include <sys/select.h>
#include <netdb.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/utilities.h>
#include <net-snmp/net-snmp-includes.h>

#include "bmon_defines.h"
#include "bmon_json.h"
#include "bmon_convert_macros.h"

#include "cm.h"

STRING_MAP_START(cmErrorNameMap)
STRING_MAP_ENTRY(CM_ERROR_NONE, "Success")
STRING_MAP_ENTRY(CM_ERROR_SNMP_OPEN_FAILED, "snmp_open() failed")
STRING_MAP_ENTRY(CM_ERROR_SNMP_PARSE_OID, "snmp_parse_oid() failed")
STRING_MAP_ENTRY(CM_ERROR_SNMP_BAD_OBJECT, "snmp encountered bad object")
STRING_MAP_ENTRY(CM_ERROR_SNMP_TIMEOUT_WAITING_FOR_PEER, "snmp timeout waiting for CM agent")
STRING_MAP_ENTRY(CM_ERROR_SNMP_SYNCH_RESPONSE, "snmp_sync_response() error")
STRING_MAP_ENTRY(CM_ERROR_TOO_MANY_OIDS_SPECIFIED, "Too many OIDs specified in URL")
STRING_MAP_ENTRY(CM_ERROR_OID_NOT_SPECIFIED, "Usage: mibs=iso.3.6.1.2.1.1.9.1.3.1, or subtree=<OID> or mib_name=/mib_type=/mib_value= triple")
STRING_MAP_ENTRY(CM_ERROR_INVALID_MIB_TYPE, "MIB type is invalid. Use i, u, 3, t, a, o, s, x, d, or b.")
STRING_MAP_ENTRY(CM_ERROR_SNMP_PACKET_ERROR, "The packet that was returned from the SNMP call was not properly formatted.")
STRING_MAP_ENTRY(CM_ERROR_APPEND_PLUGIN_HEADER_FAILED, "append_plugin_header() failed")
STRING_MAP_ENTRY(CM_ERROR_MAX, "Unknown error code")
STRING_MAP_END()

ENUM_TO_STRING_FUNCTION(cmErrorToString, cm_errors, cmErrorNameMap)
STRING_TO_ENUM_FUNCTION(stringToCmError, cm_errors, cmErrorNameMap)

#define NETSNMP_DS_APP_DONT_FIX_PDUS              0
#define NETSNMP_DS_WALK_INCLUDE_REQUESTED         1
#define NETSNMP_DS_WALK_PRINT_STATISTICS          2
#define NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC  3

static char * argv2[] = { "cm_smnp", "-cpub", "-v2c", "192.168.100.1", "iso.3.6.1.2.1.1.1.0", "iso.3.6.1.2.1.1.9.1.3.1", NULL };
static int reps       = 10;
static int non_reps   = 0;
static int numprinted = 0;

#include "cm.h"
#include "plugin_header.h"

#define ALLOC_AND_COPY(WHICH_BUFFER)                                                                                                                                                             \
    if (cm_data->mib_count < BMON_CM_MIB_MAX_NUM && cm_data->mib_name_value[cm_data->mib_count].mib_value == NULL) {                                                                             \
        calloc_len = strlen(WHICH_BUFFER) + 1;                                                                                                                                                   \
        cm_data->mib_name_value[cm_data->mib_count].mib_value = calloc(1, calloc_len);                                                                                                           \
        if (debug) { fprintf(stderr, "%s - calloc(%d) %p for (%s) \n", __FUNCTION__, calloc_len, cm_data->mib_name_value[cm_data->mib_count].mib_value, WHICH_BUFFER); }                         \
    }                                                                                                                                                                                            \
    if (cm_data->mib_count < BMON_CM_MIB_MAX_NUM && cm_data->mib_name_value[cm_data->mib_count].mib_value != NULL) {                                                                             \
        /*if (debug) {fprintf( stdout, "%s - WHICH_BUFFER[0] (%c) (%c) (%s) \n", __FUNCTION__, WHICH_BUFFER[0], WHICH_BUFFER[strlen( WHICH_BUFFER )-1], WHICH_BUFFER ); }                     */ \
        snprintf(cm_data->mib_name_value[cm_data->mib_count].mib_value, calloc_len, "%s", WHICH_BUFFER);                                                                                         \
        if (debug) { fprintf(stderr, "%s - snprintf(%s) to idx (%d) ... (%s)\n", __FUNCTION__, WHICH_BUFFER, cm_data->mib_count, cm_data->mib_name_value[cm_data->mib_count].mib_value); }       \
        cm_data->mib_count++;                                                                                                                                                                    \
    }

static void bmon_cm_optProc(
        int            argc,
        char * const * argv,
        int            opt
        )
{
    switch (opt)
    {
        fprintf(stderr, "%s opt (%c) ... optarg (%s) \n", __FUNCTION__, opt, *optarg);
    case 'C':
    {
        while (*optarg)
        {
            switch (*optarg++)
            {
            case 'f':
            {
                netsnmp_ds_toggle_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_APP_DONT_FIX_PDUS);
                break;
            }
            default:
                fprintf(stderr, "Unknown flag passed to -C: %c\n", optarg[-1]);
                return;
            }   /* switch */
        }
        break;
    }
    } /* switch */
}     /* bmon_cm_optProc */

static int create_oid_string(
        const oid *                   objid,
        size_t                        objidlen,
        const netsnmp_variable_list * variable,
        bmon_cm_t *                   cm_data
        )
{
    int               debug      = 0;
    int               rc         = 0;
    u_char *          buf        = NULL;
    size_t            buf_len    = 256, out_len = 0;
    unsigned long int calloc_len = 0;

    if (cm_data->mib_count == BMON_CM_MIB_MAX_NUM)
    {
        rc = -1;
        if (debug) { fprintf(stderr, "%s - mib_count max reached %d \n", __FUNCTION__, cm_data->mib_count); }
        return(rc);
    }

    if (debug) { fprintf(stderr, "%s - mib_count %d ... mib_max %d \n", __FUNCTION__, cm_data->mib_count, BMON_CM_MIB_MAX_NUM); }
    if ((buf = (u_char *) calloc(buf_len, 1)) == NULL)
    {
        if (debug) { fprintf(stderr, "[TRUNCATED]\n"); }
        return(rc);
    }
    else
    {
        if (sprint_realloc_variable(&buf, &buf_len, &out_len, 1, objid, objidlen, variable))
        {
            /*
             * iso.3.6.1.2.1.1.2.0 = OID: iso.3.6.1.4.1.4413.2.1.2.1.3390
             * iso.3.6.1.2.1.1.7.0 = INTEGER: 74
             * iso.3.6.1.4.1.4413.2.2.2.1.9.1.1.5.1.5.4 = STRING: "IPv6 Stack Version 1.2.0\neCos Console Cmds, (no Idle Loop Profiler)"
             */
            char * equals = strstr(buf, " = "); /* this separates the name and the type */
            char * colon  = strstr(buf, ": ");  /* this separates the type and the value */

            if (debug) { fprintf(stderr, "%s - buf (%s) ... equals %p ... colon %p \n", __FUNCTION__, buf, equals, colon); }
            /* split the line up into 3 parts ... 1) OID name 2) type and 3) value */
            if (equals && colon)
            {
                *equals = '\0';
                *colon  = '\0';
                equals += 3; /* the type field start 3 characters after the equals sign */
                colon  += 2; /* the oid value starts 2 characters after the colon */

                snprintf(cm_data->mib_name_value[cm_data->mib_count].mib_name, sizeof(cm_data->mib_name_value[0].mib_name) - 1, "%s", buf);
                snprintf(cm_data->mib_name_value[cm_data->mib_count].mib_type, sizeof(cm_data->mib_name_value[0].mib_type) - 1, "%s", equals);
                /*
                 * fprintf(stderr, "{\"oid\":\"%s\", \"type\":\"%s\", \"value\":", buf, equals );
                 * the STRING type requires special processing because of the potential of newlines being in the string
                 */
                if (strstr(equals, "STRING"))
                {
                    /* if the value string has newline in it, we need to replace binary 0xa (1 byte) with string "\n" (2 bytes) */
                    if (strchr(colon, '\n'))
                    {
                        int    idx        = 0;
                        int    newlen     = strlen(colon) + 128; /* assume there are no more than 128 newlines in the value string */
                        char * newline    = NULL;
                        char * prevline   = NULL;
                        char * longer_buf = malloc(newlen);
                        /*fprintf(stderr, "found newline ... newlen %d \n", newlen );*/
                        if (longer_buf)
                        {
                            char temp = 0;
                            prevline = colon;
                            memset(longer_buf, 0, newlen);
                            while ((newline = strchr(prevline, '\n')))
                            {
                                *newline = '\0';
                                strncat(longer_buf, prevline, newlen - strlen(longer_buf) - 1);
#if ADD_NEWLINE_CHARACTER
                                strncat(longer_buf, "\\n", newlen - strlen(longer_buf) - 1);
#endif
                                /*fprintf(stderr, "found newline partial prevline (%s) ... newlen (%d) \n", prevline, strlen(longer_buf) );*/
                                newline++;
                                prevline = newline;
                            }
                            strncat(longer_buf, prevline, newlen - strlen(longer_buf) - 1);
                            /*
                             * fprintf(stderr, "newline final (%s) ", longer_buf );
                             * fprintf(stderr, "%s}", longer_buf );
                             */
                            ALLOC_AND_COPY(longer_buf);
                            free(longer_buf);
                        }
                        else
                        {
                            fprintf(stderr, "%s}", "\"Could not malloc(%d)\"", newlen);
                        }
                    }
                    else
                    {
                        /*fprintf(stderr, "%s", colon );*/
                        ALLOC_AND_COPY(colon);
                    }
                }
                else
                {
                    if (debug) { fprintf(stderr, "%s - ALLOC_AND_COPY(%s) \n", __FUNCTION__, colon); }
                    ALLOC_AND_COPY(colon);
                    if (debug) { fprintf(stderr, "%s - ALLOC_AND_COPY(%s) done \n", __FUNCTION__, colon); }
                }
            }
            else
            {
            }
        }
        else
        {
            /* fprintf(stderr, "%s [TRUNCATED]}", buf); */
            ALLOC_AND_COPY(buf);
        }
    }

    SNMP_FREE(buf);

    if (cm_data->mib_count == BMON_CM_MIB_MAX_NUM) { rc = -1; }

    /*if (debug) {fprintf( stderr, "%s - returning %d ... mib_count %d ... mib_max %d \n", __FUNCTION__, rc, cm_data->mib_count, BMON_CM_MIB_MAX_NUM ); }*/
    return(rc);
} /* create_oid_string */

static void snmp_get_and_print(
        netsnmp_session * ss,
        oid *             theoid,
        size_t            theoid_len
        )
{
    netsnmp_pdu *           pdu, * response;
    netsnmp_variable_list * vars;
    int                     status;

    pdu = snmp_pdu_create(SNMP_MSG_GET);
    snmp_add_null_var(pdu, theoid, theoid_len);

    status = snmp_synch_response(ss, pdu, &response);
    if ((status == STAT_SUCCESS) && (response->errstat == SNMP_ERR_NOERROR))
    {
        for (vars = response->variables; vars; vars = vars->next_variable)
        {
            numprinted++;
            print_variable(vars->name, vars->name_length, vars);
        }
    }
    if (response)
    {
        snmp_free_pdu(response);
    }
} /* snmp_get_and_print */

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int cm_get_data(
        const char * filter,
        char *       json_output,
        unsigned int json_output_size,
        int          argc,
        char *       argv[]
        )
{
    int                     rc         = 0;
    cJSON *                 objectRoot = NULL;
    cJSON *                 objectData = NULL;
    bmon_cm_t               cm_data;
    int                     idx   = 0;
    int                     debug = 0;
    netsnmp_session         session, * sess = NULL;
    netsnmp_pdu *           pdu          = NULL;
    netsnmp_pdu *           response     = NULL;
    netsnmp_variable_list * vars         = NULL;
    int                     arg          = 0;
    int                     argc2        = 6;
    int                     count        = 0;
    int                     current_name = 0;
    char *                  names[SNMP_MAX_CMDLINE_OIDS];
    char *                  mibs_specified_csv = NULL;
    char *                  subtree_specified  = NULL;
    char *                  mib_name           = NULL;      /* used when setting a mib to a specific value */
    char *                  mib_type           = NULL;      /* used when setting a mib to a specific value */
    char *                  mib_value          = NULL;      /* used when setting a mib to a specific value */
    oid                     name[MAX_OID_LEN];
    size_t                  name_length = 0;
    oid                     root[MAX_OID_LEN];
    int                     status             = 0;
    int                     parse_oid_error    = 0;
    int                     exitval            = 0;
    struct timeval          tv1                = { 0, 0 }, tv2 = { 0, 0 };
    unsigned long long int  microseconds1      = 0, microseconds2 = 0;
    unsigned long int       microseconds_delta = 0;

    assert(NULL != filter);
    assert(NULL != json_output);
    assert(0 < json_output_size);

    memset(&cm_data, 0, sizeof(cm_data));

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, cm_data.status_code, CM_ERROR_APPEND_PLUGIN_HEADER_FAILED, finish);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, CM_PLUGIN_NAME, CM_PLUGIN_DESCRIPTION, NULL, CM_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, cm_data.status_code, CM_ERROR_APPEND_PLUGIN_HEADER_FAILED, finish);

    /* add plug in data */

    if (debug) { fprintf(stderr, "%s - filter (%s) ... oid size %d ... name size %d \n", __FUNCTION__, filter, sizeof(oid), sizeof(name)); } fflush(stderr);

    /* get the common command line arguments */
    arg = snmp_parse_args(argc2, argv2, &session, "C:", bmon_cm_optProc);
    /* fprintf( stderr, "after snmp_parse_args ... arg %d ... SNMP_MAX_CMDLINE_OIDS %d \n", arg, SNMP_MAX_CMDLINE_OIDS ); */

    /* fprintf( stderr, "%s - arg %d ... argc %d \n", __FUNCTION__, arg, argc );fflush(stderr); */
    if ((argc - arg) > SNMP_MAX_CMDLINE_OIDS)
    {
        fprintf(stderr, "Too many object identifiers specified. Only %d allowed in one request.\n", SNMP_MAX_CMDLINE_OIDS);
        cm_data.status_code = CM_ERROR_TOO_MANY_OIDS_SPECIFIED;
        goto finish;
    }

    /* If the user provided some OIDs to look for, then use the user's OIDs */
    mibs_specified_csv = strstr(filter, "mibs=");
    subtree_specified  = strstr(filter, "subtree=");
    mib_name           = strstr(filter, "mib_name=");
    mib_type           = strstr(filter, "mib_type=");
    mib_value          = strstr(filter, "mib_value=");

    if (mib_name && mib_type && mib_value) /* user wants to set a mib to a specific value */
    {
        int    count;
        int    current_name  = 0;
        int    current_type  = 0;
        int    current_value = 0;
        int    failures      = 0;
        char * ampersand     = NULL;
        char * names[SNMP_MAX_CMDLINE_OIDS];
        char   types[SNMP_MAX_CMDLINE_OIDS];
        char * values[SNMP_MAX_CMDLINE_OIDS];

        memset(names, 0, sizeof(names));
        memset(types, 0, sizeof(types));
        memset(values, 0, sizeof(values));

        ampersand = strchr(mib_name, '&'); if (ampersand) { *ampersand = '\0'; }
        ampersand = strchr(mib_type, '&'); if (ampersand) { *ampersand = '\0'; }
        ampersand = strchr(mib_value, '&'); if (ampersand) { *ampersand = '\0'; }

        if (debug) { fprintf(stderr, "%s - name (%s) ... type (%s) ... value (%s) \n", __FUNCTION__, &mib_name[9], &mib_type[9], &mib_value[10]); }

        names[current_name++] = &mib_name[9]; /* advance past the field name */
        do
        {
            switch (mib_type[9])
            {
            case '=':
            case 'i':
            case 'u':
            case '3':
            case 't':
            case 'a':
            case 'o':
            case 's':
            case 'x':
            case 'd':
            case 'b':
#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
            case 'I':
            case 'U':
            case 'F':
            case 'D':
#endif /* NETSNMP_WITH_OPAQUE_SPECIAL_TYPES */
                {
                    types[current_type++] = mib_type[9];
                    break;
                }
            default:
            {
                fprintf(stderr, "%s: Bad object type: %c\n", __FUNCTION__, mib_type[9]);
                cm_data.status_code = CM_ERROR_INVALID_MIB_TYPE;
                break;
            }
            } /* switch */

            values[current_value++] = &mib_value[10];

            SOCK_STARTUP;

            /* open an SNMP session */
            sess = snmp_open(&session);
            if (sess == NULL)
            {
                /* diagnose snmp_open errors with the input netsnmp_session pointer */
                snmp_sess_perror("snmpset", &session);
                SOCK_CLEANUP;
                cm_data.status_code = CM_ERROR_SNMP_OPEN_FAILED;
                break;
            }

            /* create PDU for SET request and add object names and values to request */
            pdu = snmp_pdu_create(SNMP_MSG_SET);
            for (count = 0; count < current_name; count++)
            {
                name_length = MAX_OID_LEN;
                if (snmp_parse_oid(names[count], name, &name_length) == NULL)
                {
                    snmp_perror(names[count]);
                    failures++;
                    break;
                }
                else
                if (snmp_add_var(pdu, name, name_length, types[count], values[count]))
                {
                    snmp_perror(names[count]);
                    failures++;
                    break;
                }
            }

            if (failures)
            {
                snmp_close(sess);
                SOCK_CLEANUP;
                cm_data.status_code = CM_ERROR_SNMP_PARSE_OID;
                break;
            }

            /* do the request */
            status = snmp_synch_response(sess, pdu, &response);
            if (status == STAT_SUCCESS)
            {
                if (response->errstat == SNMP_ERR_NOERROR)
                {
                    if (debug)
                    {
                        for (vars = response->variables; vars; vars = vars->next_variable)
                        {
                            print_variable(vars->name, vars->name_length, vars);
                        }
                    }
                    /* artificially set the mibs_specified_csv var to instruct the block of code after to this to get the current value of the MIB that we just set */
                    mibs_specified_csv = &mib_name[4]; /* mib_name=iso.3.6.1.5 ... mibs=iso.3.6.1.5 */
                }
                else
                {
                    fprintf(stderr, "Error in packet.\nReason: %s\n", snmp_errstring(response->errstat));
                    if (response->errindex != 0)
                    {
                        fprintf(stderr, "Failed object: ");
                        for (count = 1, vars = response->variables; vars && (count != response->errindex); vars = vars->next_variable, count++)
                        {
                        }
                        if (vars)
                        {
                            fprint_objid(stderr, vars->name, vars->name_length);
                        }
                        fprintf(stderr, "\n");
                    }
                    cm_data.status_code = CM_ERROR_SNMP_PACKET_ERROR;
                    break;
                }
            }
            else
            if (status == STAT_TIMEOUT)
            {
                fprintf(stderr, "Timeout: No Response from %s\n", session.peername);
                cm_data.status_code = CM_ERROR_SNMP_TIMEOUT_WAITING_FOR_PEER;
                break;
            }
            else /* status == STAT_ERROR */
            {
                snmp_sess_perror("snmpset", sess);
                cm_data.status_code = CM_ERROR_SNMP_SYNCH_RESPONSE;
                break;
            }

            if (response) { snmp_free_pdu(response); }
            snmp_close(sess);
            SOCK_CLEANUP;
        }
        while (0);
    }

    if (mibs_specified_csv)
    {
        char * comma    = NULL;
        char * oid_name = NULL;

        oid_name     = mibs_specified_csv + 5; /* advance past the beginning tag string ... should now be pointing to first OID */
        current_name = 0;                      /* start over from the beginning of the names list */

        if (debug) { fprintf(stderr, "%s - mibs=oid_name is (%p) \n", __FUNCTION__, oid_name); } fflush(stderr);
        do
        {
            if (oid_name)
            {
                comma = strchr(oid_name, ',');
                if (comma) { *comma = '\0'; }
                names[current_name] = oid_name;
                if (debug) { fprintf(stderr, "%s - names[%d] is %p (%s) \n", __FUNCTION__, current_name, oid_name, oid_name); } fflush(stderr);
                current_name++;

                if (comma) { oid_name = ++comma; }

                if (debug) { fprintf(stderr, "%s - comma (%p) ---------------------------------------------------------------------------- \n", __FUNCTION__, comma); } fflush(stderr);
            }
        }
        while (comma);

        do
        {
            SOCK_STARTUP;

            if (debug) { fprintf(stderr, "%s - snmp_open() ... current_name %d \n", __FUNCTION__, current_name); } fflush(stderr);
            /* Open an SNMP session.  */
            sess = snmp_open(&session);
            if (sess == NULL)
            {
                /* diagnose snmp_open errors with the input netsnmp_session pointer */
                SOCK_CLEANUP;
                cm_data.status_code = CM_ERROR_SNMP_OPEN_FAILED;
                break;
            }

            /* Create PDU for GET request and add object names to request.  */
            pdu = snmp_pdu_create(SNMP_MSG_GET);
            for (count = 0; count < current_name && names[count]; count++)
            {
                name_length = MAX_OID_LEN;
                if (debug) { fprintf(stderr, "%s - %u ... snmp_parse_oid (names[%d] -> %s) \n", __FUNCTION__, __LINE__, count, names[count]); } fflush(stderr);
                memset(&name, 0, sizeof(name));
                if (!snmp_parse_oid(names[count], name, &name_length))
                {
                    fprintf(stderr, "%s - %u ... snmp_parse_oid error \n", __FUNCTION__, __LINE__); fflush(stderr);
                    snmp_perror(names[count]);
                    parse_oid_error++;
                }
                else
                {
                    /* fprintf( stderr, "%s - %u ... parse okay ... snmp_add_null_var() \n", __FUNCTION__, __LINE__ );fflush(stderr);*/
                    snmp_add_null_var(pdu, name, name_length);
                }
            }

            if (debug) { fprintf(stderr, "%s - %u ... parse_oid_error %d \n", __FUNCTION__, __LINE__, parse_oid_error); } fflush(stderr);
            if (parse_oid_error)
            {
                snmp_close(sess);
                SOCK_CLEANUP;
                cm_data.status_code = CM_ERROR_SNMP_PARSE_OID;
                break;
            }

retry:
            /* send the request to the CM agent and read the response */
            if (debug) { fprintf(stderr, "%s - snmp_synch_response() \n", __FUNCTION__); } fflush(stderr);
            status = snmp_synch_response(sess, pdu, &response);
            if (debug) { fprintf(stderr, "%s - snmp_synch_response() returned %d \n", __FUNCTION__, status); }

            if (status == STAT_SUCCESS)
            {
                if (response->errstat == SNMP_ERR_NOERROR)
                {
                    for (vars = response->variables; vars; vars = vars->next_variable)
                    {
                        create_oid_string(vars->name, vars->name_length, vars, &cm_data);
                        if (debug) { fprintf(stderr, "%s - create_oid_string(%s) mib_count %d \n", __FUNCTION__, vars->name, cm_data.mib_count); }
                    }
                }
                else
                {
                    /* If the request fails, remove the OID that had an error. Then retry. */
                    fprintf(stderr, "Error in packet\nReason: %s\n", snmp_errstring(response->errstat));

                    if (response->errindex != 0)
                    {
                        fprintf(stderr, "Failed object: ");
                        for (count = 1, vars = response->variables; vars && count != response->errindex; vars = vars->next_variable, count++)
                        {
                            /*EMPTY*/
                        }
                        if (vars)
                        {
                            fprint_objid(stderr, vars->name, vars->name_length);
                        }
                        fprintf(stderr, "\n");
                    }
                    cm_data.status_code = CM_ERROR_SNMP_BAD_OBJECT;

                    /* retry if the errored variable was successfully removed */
                    if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_APP_DONT_FIX_PDUS))
                    {
                        pdu = snmp_fix_pdu(response, SNMP_MSG_GET);
                        snmp_free_pdu(response);
                        response = NULL;
                        if (pdu != NULL)
                        {
                            goto retry;
                        }
                    }
                } /* endif -- SNMP_ERR_NOERROR */
            }
            else
            if (status == STAT_TIMEOUT)
            {
                fprintf(stderr, "Timeout: No Response from %s.\n", session.peername);
                cm_data.status_code = CM_ERROR_SNMP_TIMEOUT_WAITING_FOR_PEER;
            }
            else /* status == STAT_ERROR */
            {
                snmp_sess_perror("snmpget", sess);
                cm_data.status_code = CM_ERROR_SNMP_SYNCH_RESPONSE;
            } /* endif -- STAT_SUCCESS */

            if (response)
            {
                snmp_free_pdu(response);
            }
            snmp_close(sess);
            SOCK_CLEANUP;
        }
        while (0);
    }
    else
    if (subtree_specified)
    {
        char * comma    = NULL;
        char * oid_name = NULL;
        size_t rootlen  = MAX_OID_LEN;
        int    running  = 0;
        int    check    = 0;

        gettimeofday(&tv1, NULL);
        microseconds1  = (tv1.tv_sec * 1000000LL); /* q-scale shift the seconds left to allow for addition of microseconds */
        microseconds1 += tv1.tv_usec;              /* add in microseconds */

        netsnmp_ds_register_config(ASN_BOOLEAN, "cm_plugin", "includeRequested", NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_INCLUDE_REQUESTED);
        netsnmp_ds_register_config(ASN_BOOLEAN, "cm_plugin", "printStatistics", NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_PRINT_STATISTICS);
        netsnmp_ds_register_config(ASN_BOOLEAN, "cm_plugin", "dontCheckOrdering", NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC);

        oid_name = subtree_specified + 8; /* advance past the beginning tag string ... should now be pointing to first OID */

        rootlen = MAX_OID_LEN;
        if (snmp_parse_oid(oid_name, root, &rootlen) == NULL)
        {
            snmp_perror(oid_name);
            exit(1);
        }
        /*fprintf( stderr, "%s - root (%s) ... rootlen (%d) \n", argv[0], root, rootlen );fflush(stderr);*/
        if (debug) { fprintf(stderr, "%s - subtree=oid_name is (%s) \n", __FUNCTION__, oid_name); } fflush(stderr);

        do
        {
            SOCK_STARTUP;

            if (debug) { fprintf(stderr, "%s - snmp_open() \n", __FUNCTION__); } fflush(stderr);
            /* Open an SNMP session.  */
            sess = snmp_open(&session);
            if (sess == NULL)
            {
                /* diagnose snmp_open errors with the input netsnmp_session pointer */
                SOCK_CLEANUP;
                cm_data.status_code = CM_ERROR_SNMP_OPEN_FAILED;
                break;
            }

            memset(name, 0, sizeof(name));

            if (debug) { fprintf(stderr, "%s - memmove(%p, %s, len %d) \n", __FUNCTION__, name, oid_name, rootlen * sizeof(oid)); } fflush(stderr);
            strncpy((char *) &name, oid_name, MAX_OID_LEN-1);
            name_length = rootlen;

            /* setup initial object name */
            memmove(name, root, rootlen * sizeof(oid));
            name_length = rootlen;

            running = 1;

            /*fprintf(stderr, "%s - netsnmp_ds_get_boolean(NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC) \n", __FUNCTION__ );fflush(stderr);*/
            check = !netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_DONT_CHECK_LEXICOGRAPHIC);

            /*fprintf(stderr, "%s - netsnmp_ds_get_boolean(NETSNMP_DS_WALK_INCLUDE_REQUESTED) \n", __FUNCTION__ );fflush(stderr);*/
            if (0 /* CAD */ && netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_INCLUDE_REQUESTED))
            {
                snmp_get_and_print(sess, root, rootlen);
            }

            while (running)
            {
                /* create PDU for GETBULK request and add object name to request */
                if (debug) { fprintf(stderr, "%s - snmp_pdu_create(SNMP_MSG_GETBULK) \n", __FUNCTION__); } fflush(stderr);
                pdu                  = snmp_pdu_create(SNMP_MSG_GETBULK);
                pdu->non_repeaters   = non_reps;
                pdu->max_repetitions = reps; /* fill the packet */
                snmp_add_null_var(pdu, name, name_length);

                /* do the request */
                if (debug) { fprintf(stderr, "%s - subtree snmp_synch_response() \n", __FUNCTION__); } fflush(stderr);
                status = snmp_synch_response(sess, pdu, &response);

                gettimeofday(&tv2, NULL);
                microseconds2      = (tv2.tv_sec * 1000000LL); /* q-scale shift the seconds left to allow for addition of microseconds */
                microseconds2     += tv2.tv_usec;              /* add in microseconds */
                microseconds_delta = (microseconds2 - microseconds1);
                if (debug) { fprintf(stderr, "%s - status %d ... snmp_synch_response elapsed time %lu milliseconds\n", __FUNCTION__, status, microseconds_delta/1000); }

                if (status == STAT_SUCCESS)
                {
                    if (response->errstat == SNMP_ERR_NOERROR)
                    {
                        int rc = 0;
                        /* check resulting variables */
                        for (vars = response->variables; vars; vars = vars->next_variable)
                        {
                            if ((vars->name_length < rootlen) || (memcmp(root, vars->name, rootlen * sizeof(oid)) != 0))
                            {
                                /* not part of this subtree */
                                running = 0;
                                continue;
                            }
                            numprinted++;
#if 1
                            /*if (debug) {fprintf( stderr, "%s - subtree create_oid_string() \n", __FUNCTION__ ); } fflush( stderr );*/
                            rc = create_oid_string(vars->name, vars->name_length, vars, &cm_data);
                            if (debug) { fprintf(stderr, "%s - subtree create_oid_string() done ... rc %d \n", __FUNCTION__, rc); } fflush(stderr);
                            if (rc < 0) /* if the resulting array has filled up, stop requesting more OIDs */
                            {
                                running = 0;
                                break; /*continue;*/
                            }
#else /* if 1 */
                            print_variable(vars->name, vars->name_length, vars);
#endif /* if 1 */
                            if ((vars->type != SNMP_ENDOFMIBVIEW) &&
                                (vars->type != SNMP_NOSUCHOBJECT) &&
                                (vars->type != SNMP_NOSUCHINSTANCE))
                            {
                                /*
                                 * not an exception value
                                 */
                                if (check &&
                                    (snmp_oid_compare(name, name_length,
                                             vars->name,
                                             vars->name_length) >= 0))
                                {
                                    fprintf(stderr, "Error: OID not increasing: ");
                                    fprint_objid(stderr, name, name_length);
                                    fprintf(stderr, " >= ");
                                    fprint_objid(stderr, vars->name,
                                            vars->name_length);
                                    fprintf(stderr, "\n");
                                    running = 0;
                                    exitval = 1;
                                }
                                /*
                                 * Check if last variable, and if so, save for next request.
                                 */
                                if (vars->next_variable == NULL)
                                {
                                    memmove(name, vars->name,
                                            vars->name_length * sizeof(oid));
                                    name_length = vars->name_length;
                                }
                            }
                            else
                            {
                                /*
                                 * an exception value, so stop
                                 */
                                running = 0;
                            }
                        }
                    }
                    else
                    {
                        /*
                         * error in response, print it
                         */
                        running = 0;
                        if (response->errstat == SNMP_ERR_NOSUCHNAME)
                        {
                            printf("End of MIB\n");
                        }
                        else
                        {
                            fprintf(stderr, "Error in packet.\nReason: %s\n", snmp_errstring(response->errstat));
                            if (response->errindex != 0)
                            {
                                fprintf(stderr, "Failed object: ");
                                for (count = 1, vars = response->variables;
                                     vars && count != response->errindex;
                                     vars = vars->next_variable, count++)
                                {
                                    /*EMPTY*/ }
                                if (vars)
                                {
                                    fprint_objid(stderr, vars->name, vars->name_length);
                                }
                                fprintf(stderr, "\n");
                            }
                            exitval = 2;
                        }
                    }
                }
                else
                if (status == STAT_TIMEOUT)
                {
                    fprintf(stderr, "Timeout: No Response from %s\n", session.peername);
                    running = 0;
                    exitval = 1;
                }
                else /* status == STAT_ERROR */
                {
                    snmp_sess_perror("cm_plugin", sess);
                    running = 0;
                    exitval = 1;
                }
                if (response)
                {
                    snmp_free_pdu(response);
                }
            }

            if (0 /* CAD */ && (numprinted == 0) && (status == STAT_SUCCESS))
            {
                /*
                 * no printed successful results, which may mean we were
                 * pointed at an only existing instance.  Attempt a GET, just
                 * for get measure.
                 */
                snmp_get_and_print(sess, root, rootlen);
            }
            snmp_close(sess);

            if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_WALK_PRINT_STATISTICS))
            {
                printf("Variables found: %d\n", numprinted);
            }

            SOCK_CLEANUP;

            gettimeofday(&tv2, NULL);
            microseconds2      = (tv2.tv_sec * 1000000LL); /* q-scale shift the seconds left to allow for addition of microseconds */
            microseconds2     += tv2.tv_usec;              /* add in microseconds */
            microseconds_delta = (microseconds2 - microseconds1);
            /*if(debug) fprintf( stderr, "%s - total app elapsed time           %lu milliseconds\n", __FUNCTION__, microseconds_delta/1000 );*/
        }
        while (0);
    }
    else
    {
        cm_data.status_code = CM_ERROR_OID_NOT_SPECIFIED;
        goto finish;
    }

    if (cm_data.status_code == CM_ERROR_NONE)
    {
        /* check filter once for entire array */
        if (true == json_CheckFilter(objectData, filter, objectData, "mibsArray"))
        {
            cJSON * mibArray = NULL;
            int     i        = 0;

            mibArray = json_AddArray(objectData, NO_FILTER, objectData, "mibsArray");
            CHECK_PTR_GOTO(mibArray, finish);

            while (i < BMON_CM_MIB_MAX_NUM && strlen(cm_data.mib_name_value[i].mib_name))
            {
                cJSON * objectItem = NULL;

                /* create object to be an array item which contains multiple objects */
                objectItem = json_AddArrayElement(mibArray);
                CHECK_PTR_GOTO(objectItem, finish);

                json_AddString(objectItem, NO_FILTER, objectData, "mibName", cm_data.mib_name_value[i].mib_name);
                json_AddString(objectItem, NO_FILTER, objectData, "mibType", cm_data.mib_name_value[i].mib_type);
                json_AddString(objectItem, NO_FILTER, objectData, "mibValue", cm_data.mib_name_value[i].mib_value);

                i++;
            }
        }
    }

finish:
    {
        cJSON * objectItem = NULL;

        /* create object to be an array item which contains multiple objects */
        objectItem = json_AddArrayElement(objectData);
        CHECK_PTR_GOTO(objectItem, skipStatus);

        json_AddNumber(objectItem, filter, objectData, "statusCode", cm_data.status_code);
        json_AddString(objectItem, filter, objectData, "statusString", cmErrorToString(cm_data.status_code));
    }
skipStatus:

    /* clean up ... there could be some string pointers that were allocated during the process */
    idx = 0;
    while (cm_data.mib_name_value[idx].mib_value)
    {
        free(cm_data.mib_name_value[idx].mib_value);
        /* fprintf(stderr, "%s - free( mib_value[%d] -> %p) \n", __FUNCTION__, idx, cm_data.mib_name_value[idx].mib_value );*/
        idx++;
    }

    /* copy JSON data to supplied buffer */
    rc = json_Print(objectRoot, json_output, json_output_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    json_Uninitialize(&objectRoot);

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen(json_output);
    }

    return(rc);
} /* cm_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (1024 * 10)
/**
 * Function: This function will coordinate collecting cm data and once that is done,
 * it will convert the cm data to a JSON format and send the JSON data back
 * to the browser or curl or wget.
 **/
int main(
        int    argc,
        char * argv[]
        )
{
    int    ret             = 0;
    char   filterDefault[] = "/";
    char * pFilter         = filterDefault;
    char   payload[PAYLOAD_SIZE];

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    ret = cm_get_data(pFilter, payload, PAYLOAD_SIZE, argc, argv);
    CHECK_ERROR_GOTO("Failure getting CM data", ret, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:
    return(ret);
} /* main */

#endif /* defined(BMON_PLUGIN) */