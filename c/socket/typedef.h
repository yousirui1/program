#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#pragma pack(4)

typedef int           BOOL;         /*  Signed  32 bit as bool */
typedef unsigned char UCHAR8;       /* Unsigned  8 bit quantity */
typedef signed char CHAR8;          /* Signed    8 bit quantity */
typedef unsigned short USHORT16;    /* Unsigned 16 bit quantity */
typedef signed short SHORT16;       /* Signed   16 bit quantity */
typedef unsigned int UINT32;       /* Unsigned 32 bit quantity */
typedef signed int SINT32;         /* Signed   32 bit quantity */
typedef signed int SLONG32;
typedef unsigned int ULONG32;
typedef long long SLONG64;          /* signed 64 bit quantity */
typedef unsigned short BYTE;        /* 16bit */
typedef unsigned short WORD;        /* 16bit */ 
typedef unsigned int DWORD;         /* 32bit */
typedef unsigned int FOURCC;        /* 32bit */

#define STATE_OFF 0
#define STATE_ON  1

#define FALSE     0
#define TRUE      1

#define __DEBUG__
#ifdef __DEBUG__
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format,...)
#endif

#endif
