#ifndef DSGCLIENTCALLBACK_H
#define DSGCLIENTCALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

extern int dsgClientCallbackInit();
extern int dsgClientCallbackSvcRun();

extern  int * twowaymode_1_svc(int *, struct svc_req *);
extern  int * advancedmode_1_svc(int *, struct svc_req *);
extern  int * tunnelpresent_1_svc(int *, struct svc_req *);

#ifdef __cplusplus
}
#endif

#endif /* !DSGCLIENTCALLBACK_H */
