#include <net-snmp/net-snmp-config.h>

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "snmpTargetAddrEntry.h"
#include "snmpTargetParamsEntry.h"
#include "target.h"

//#define MAX_TAGS 128

/* BRCM_MOD - PR6554 Bug fixes here. Note that we have our own bcmvV2_get_target_sessions
routine; but, other netsnmp routines call this one.  We should probably fix them to
call ours.  For now just patch this one.

Note: the spec says we need to only support 10 TLV-38s: section 3.6 docsisV3NotificationReceiver */
#define _MAX_TAGS 10

netsnmp_session *
get_target_sessions(char *taglist, TargetFilterFunction * filterfunct,
                    void *filterArg)
{
    char mybuf[14];
    netsnmp_session *ret = NULL, thissess;
    struct targetAddrTable_struct *targaddrs;
    //char            buf[SPRINT_MAX_LEN];
    //char            tags[MAX_TAGS][SPRINT_MAX_LEN],  /* BRCM_MOD - whew, this was a lot of data */
    char            *cp;
    char            buf[255];
    static char     tags[_MAX_TAGS][255];
    
    int             numtags = 0, i;
    static struct targetParamTable_struct *param;

    printf("XXXXX The real get_target_sessions I want to know if we get here! XXX\n");  
    
    DEBUGMSGTL(("target_sessions", "looking for: %s\n", taglist));
    for (cp = taglist; cp && numtags < _MAX_TAGS;) {
        cp = copy_nword(cp, tags[numtags], sizeof(tags[numtags]));
        DEBUGMSGTL(("target_sessions", " for: %d=%s\n", numtags,
                    tags[numtags]));
        numtags++;
    }

    for (targaddrs = get_addrTable(); targaddrs;
         targaddrs = targaddrs->next) {

        /*
         * legal row? 
         */
        if (targaddrs->tDomain == NULL ||
            targaddrs->tAddress == NULL ||
            targaddrs->rowStatus != SNMP_ROW_ACTIVE) {
            DEBUGMSGTL(("target_sessions", "  which is not ready yet\n"));
            continue;
        }

        if (netsnmp_tdomain_support
            (targaddrs->tDomain, targaddrs->tDomainLen, NULL, NULL) == 0) {
            snmp_log(LOG_ERR,
                     "unsupported domain for target address table entry %s\n",
                     targaddrs->name);
            /* BRCM_MOD - PR6554, add continue here - if no good domain, no good row */
            continue;
        }

        /*
         * check tag list to see if we match 
         */
        if (targaddrs->tagList) {
            /*
             * loop through tag list looking for requested tags 
             */
            for (cp = targaddrs->tagList; cp;) {
                cp = copy_nword(cp, buf, sizeof(buf));
                for (i = 0; i < numtags; i++) {
                    if (strcmp(buf, tags[i]) == 0) {
                        /*
                         * found a valid target table entry 
                         */
                        DEBUGMSGTL(("target_sessions", "found one: %s\n",
                                    tags[i]));

                        // we have closed the session elsewhere (send_bcmV2_notifications)
                        // so the ptr is invalid. Make sure we don't have a sess.
                        //printf(" bcmV2_get_target_sessions: make sure target sess is NULL\n");
                        targaddrs->sess = NULL;

                        
                        if (targaddrs->sess)
                        {
                            //PR 6994: BRCM_MOD 
                            // This function returns a list of session pointers to send a notify to.
                            // This function is called once for each tag, e.g., once for @config_inform,
                            // once for @config_trap.  For example, if there are three rows with tag
                            // @config_trap, it returns a sess pointer with all three sessions for all 
                            // three rows (sess pointer has a next*).  If this has been done once, the 
                            // sessions may have a link to another session in it's next*.  This may cause
                            // a problem, especially if a row is not active anymore.  So, just set the
                            // next* to NULL.  We are not deleting sessions, just unlinking them.
                            // printf("setting targaddrs->sess->next to NULL \n");
                            targaddrs->sess->next = NULL;
                        }

                        if (targaddrs->params) {
                            param = get_paramEntry(targaddrs->params);
                            if (!param
                                || param->rowStatus != SNMP_ROW_ACTIVE) {
                                /*
                                 * parameter entry must exist and be active 
                                 */
                                continue;
                            }
                        } else {
                            /*
                             * parameter entry must be specified 
                             */
                            continue;
                        }

                        /*
                         * last chance for caller to opt-out.  Call
                         * filtering function 
                         */
                        if (filterfunct &&
                            (*(filterfunct)) (targaddrs, param,
                                              filterArg)) {
                            continue;
                        }

                        if (targaddrs->storageType != ST_READONLY &&
                            targaddrs->sess &&
                            param->updateTime >=
                            targaddrs->sessionCreationTime) {
                            /*
                             * parameters have changed, nuke the old session 
                             */
                            ////////////////////////
                            if (targaddrs->storageType != ST_READONLY)
                                printf(" 1212 targaddrs->storageType not readonly\n");
                            if (targaddrs->sess)
                                printf(" 1212 targaddrs->sess\n");
                            if (param->updateTime >= targaddrs->sessionCreationTime) 
                                printf(" 1212 uptime>sessCreation time\n");
                            
                            ////////////////////////
                            printf("1209 parameters have changed nuke old session -WHY ARE WE DOING THIS?\n");
                            snmp_close(targaddrs->sess);
                            targaddrs->sess = NULL;
                        }
                        /*
                         * target session already exists? 
                         */
                        if (targaddrs->sess == NULL) {
                            /*
                             * create an appropriate snmp session and add
                             * it to our return list 
                             */
                            netsnmp_transport *t = NULL;

                            printf("DEBUG target.c  targaddrs->tAddressLen = <%d>\n",targaddrs->tAddressLen);
                                                        
                            DEBUGMSGOID(("my domain udp",
                                             targaddrs->tDomain,
                                             targaddrs->tDomainLen));

                            mybuf[0]  = 0x0A;   
                            mybuf[1]  = 0x18;
                            mybuf[2]  = 0x05;
                            mybuf[3]  = 0x69;
                            mybuf[4]  = 0x00;
                            mybuf[5]  = 0xA3;  //try 163 for a test //dont specify a port
                            mybuf[6]  = 0x0A;
                            mybuf[7]  = 0x18;
                            mybuf[8]  = 0x41;
                            mybuf[9]  = 0x24;
                            mybuf[10] = 0x00;
                            mybuf[11] = 0xA2;
                            
                            
                            printf("calling netsnmp_tdomain_transport_oid\n");

                            t = netsnmp_tdomain_transport_oid(targaddrs->tDomain,
                                                             targaddrs->tDomainLen,
                                                             (char *)mybuf,//targaddrs->tAddress,
                                                             12,//targaddrs->tAddressLen,
                                                             0);
                                                        
                            if (t == NULL) {
                                DEBUGMSGTL(("target_sessions",
                                            "bad dest \""));
                                DEBUGMSGOID(("target_sessions",
                                             targaddrs->tDomain,
                                             targaddrs->tDomainLen));
                                DEBUGMSG(("target_sessions", "\", \""));
                                DEBUGMSGHEX(("target_sessions",
                                             targaddrs->tAddress,
                                             targaddrs->tAddressLen));
                                DEBUGMSG(("target_sessions", "\n"));
                                continue;
                            } else {
                                char           *dst_str =
                                    t->f_fmtaddr(t, NULL, 0);
                                if (dst_str != NULL) {
                                    DEBUGMSGTL(("target_sessions",
                                                "  to: %s\n", dst_str));
                                    free(dst_str);
                                }
                            }
                            memset(&thissess, 0, sizeof(thissess));
                            thissess.timeout = (targaddrs->timeout) * 1000;
                            thissess.retries = targaddrs->retryCount;
                            DEBUGMSGTL(("target_sessions",
                                        "timeout: %d -> %d\n",
                                        targaddrs->timeout,
                                        thissess.timeout));

                            if (param->mpModel == SNMP_VERSION_3 &&
                                param->secModel != 3) {
                                snmp_log(LOG_ERR,
                                         "unsupported model/secmodel combo for target %s\n",
                                         targaddrs->name);
                                /*
                                 * XXX: memleak 
                                 */
                                netsnmp_transport_free(t);
                                continue;
                            }
                            thissess.version = param->mpModel;
                            printf("1209 thissess.version <%d> \n",thissess.version);
                            
                            if (param->mpModel == SNMP_VERSION_3) {
                                thissess.securityName = param->secName;
                                thissess.securityNameLen =
                                    strlen(thissess.securityName);
                                thissess.securityLevel = param->secLevel;
                            } else {
                                thissess.community =
                                    (u_char *) strdup(param->secName);
                                thissess.community_len =
                                    strlen((char *) thissess.community);
                            }


                            printf("CREATE TARGET SESSION <%p>:\n",&thissess);
                            printf("   snmp version = <%d>\n",param->mpModel);
                            printf(" security level = <%d>\n",thissess.securityLevel);
                            printf(" security name  = <%s>\n",thissess.securityName);
                            printf(" security model = <%d>\n",thissess.securityModel);

                            //brcm_mod add the security model
                            thissess.securityModel = 3;

                            targaddrs->sess = snmp_add(&thissess, t,
                                                       NULL, NULL);
                            targaddrs->sessionCreationTime = time(NULL);
                        }
                        if (targaddrs->sess) {
                            if (ret) {
                                targaddrs->sess->next = ret;
                            }
                            ret = targaddrs->sess;
                        } else {
                            snmp_sess_perror("target session", &thissess);
                        }
                    }
                }
            }
        }
    }
    return ret;
}
