#ifndef __UTILITY_H__
#define __UTILITY_H__

typedef void (*wlLibPrep_t) (HANDLE in, HANDLE out);
typedef int (*wlLib_t) (char *str);
wlLibPrep_t wlLibPrep;
wlLib_t wlLib;

static void *thread;

HINSTANCE loadWlDll();
void stopWlDll(HINSTANCE dllHndl);
bool wlInit(HINSTANCE dllHndl, char *dllStartCmd);

void* createThread(void *(func)(void*), void *arg);
void* closeThread (void *threadid);
void killThread (void *threadid);
void wlThread();
void* threadStart(void *arg);
void threadStop();

bool command(const char *cmd, char *data);
int write (const char *data);
int read (char *data);
int writeBuffer (const char *buffer, int length);
int readBuffer(char *data);
int initIPC();
void deInitIPC();
void setupIPC();

#define READ_HANDLE 0
#define WRITE_HANDLE 1

int msgq_create(HANDLE handle[], LPCWSTR qName);
int msgq_close (HANDLE handle);
int msgq_read (HANDLE fd, void *buffer, DWORD count);
int msgq_write (HANDLE fd, const void *buffer, DWORD count);
//HANDLE hReadHandle
#define CMDPROMPT "> "

#endif
