
#ifndef __BCM_MFG_COMMON_H__
#define __BCM_MFG_COMMON_H__



#ifdef _LINUX 
#define DLLExport ""   
#endif

#ifdef __cplusplus
#ifndef FALSE
#define FALSE	false
#endif
#ifndef TRUE
#define TRUE	true
#endif
#endif

typedef unsigned char			UINT8;
typedef signed char				INT8;
typedef	unsigned short int	    UINT16;
typedef signed short int        INT16;
typedef unsigned int			UINT32;
typedef signed int              INT32;
typedef float                   FLOAT32;
typedef double					FLOAT64;

#endif /* __BCM_MFG_COMMON_H__ */
