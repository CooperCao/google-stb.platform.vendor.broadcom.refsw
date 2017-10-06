#ifndef _wmiprocs_h_
#define _wmiprocs_h_

#ifdef __cplusplus
extern "C" {
#endif

#define WMIAPI __stdcall

typedef PVOID WMIHANDLE, *PWMIHANDLE;

//
// Data consumer apis
typedef ULONG
(WMIAPI *WmiOpenBlockProc)(
    IN GUID *Guid,
    IN ULONG DesiredAccess,
    OUT WMIHANDLE *DataBlockHandle
);

typedef ULONG
(WMIAPI *WmiCloseBlockProc)(
    IN WMIHANDLE DataBlockHandle
);

typedef ULONG
(WMIAPI *WmiQueryAllDataProc)(
    IN WMIHANDLE DataBlockHandle,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
    );


typedef ULONG
(WMIAPI *WmiQuerySingleInstanceProc)(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN OUT ULONG *BufferSize,
    OUT PVOID Buffer
    );


typedef ULONG
(WMIAPI *WmiSetSingleInstanceProc)(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN ULONG Reserved,
    IN ULONG ValueBufferSize,
    IN PVOID ValueBuffer
    );

#ifdef __cplusplus
}
#endif


#endif // _wmiprocs_h_
