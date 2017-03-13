/**
 * Copyright @ 2012 Pioneer Suntec Electronic Technology Co.,LTD (PSET)
 * All Rights Reserved.
 */
/**
 * @file NCTypesDefine.h
 * \brief
 *
 */

#ifndef NCTYPESDEFINE_H
#define NCTYPESDEFINE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <ctype.h>
#include <wchar.h>
#include <errno.h>
//#include <unistd.h>
//#include <utime.h>
#include <sys/types.h>
//#include <syscall.h>
//#include <stdint.h>
//#include <bits/wordsize.h>

#include <WTypes.h>

namespace nutshell {

    ///////////////////////////////////////////////////////////
    ///Basical types define

    typedef signed char 	        int8_t;
    typedef unsigned char 	        uint8_t;
    typedef signed int 	            int16_t;
    typedef unsigned int 	        uint16_t;
    typedef signed long int 	    int32_t;
    typedef unsigned long int 	    uint32_t;
    typedef signed long long int 	int64_t;
    typedef unsigned long long int 	uint64_t;
    //typedef int16_t 	            intptr_t;
    //typedef uint16_t 	            uintptr_t;
    typedef int64_t 	intmax_t;
    typedef uint64_t 	uintmax_t;
#define nullptr  NULL


    //typedef signed char      BOOL;
    typedef unsigned char    BYTE;
    typedef int              INT;
    typedef short            SHORT;
    typedef unsigned int     UINT;
    typedef unsigned short   UINT16;
    typedef unsigned short   USHORT, WORD;
    typedef unsigned long    ULONG, DWORD, *PDWORD;
    typedef long             LONG;
    //typedef uint32_t         UINT32;
    typedef int               INT32;
    //typedef int64_t          INT64;
#if __WORDSIZE == 32
    typedef int64_t          LONGLONG;
    typedef uint64_t         ULONGLONG;
#else
    typedef long long int    		LONGLONG;
    typedef unsigned long long int	ULONGLONG;
#endif
    typedef float            FLOAT;
    typedef double           DOUBLE;

    typedef void            *PVOID;
    typedef void            *LPVOID;
    typedef const void      *LPCVOID;
    typedef BYTE            *PBYTE;
    typedef INT             *PINT;

#ifndef CONST
#   define CONST const
#endif

#ifndef VOID
#   define VOID  void
#endif

    typedef char          CHAR;
    typedef unsigned char UCHAR;
    typedef UINT16        UCHAR16;
    typedef wchar_t       UCHAR32;
    typedef char         *PCHAR, *LPSTR, *PSTR;
    typedef const CHAR   *LPCSTR;
    typedef wchar_t       WCHAR; ///< please use UCHAR32 instead of WCHAR.
    //typedef WCHAR        *LPCWSTR, *PCWSTR;

    ///////////////////////////////////////////////////////////
    ///Const definitions

//#define MAXWORD  0xffff
//#define MAXLONG  0x7fffffffL
//#define INFINITE 0xffffffffL

#ifndef MAX_PATH
#   define MAX_PATH 260
#endif
#ifndef _MAX_PATH
#   define _MAX_PATH MAX_PATH
#endif

#ifndef PAI
#   define PAI (3.141592654F)
#endif

#ifndef FALSE
#   define FALSE (0)
#endif

#ifndef TRUE
#   define TRUE  (1)
#endif

#ifndef NC_INVALID_ADDRESS
#   define NC_INVALID_ADDRESS ((void*)(-1))
#endif

    ///////////////////////////////////////////////////////////
    ///nutshell types define

    typedef long NC_ERROR;
    const NC_ERROR NC_NOERROR = 0;
    
    typedef bool NC_BOOL;
    const NC_BOOL NC_TRUE  = true;
    const NC_BOOL NC_FALSE = false;


    typedef char NCCHAR;
#define NCTEXT(C) C
#define nc_strcat      strcat
#define nc_strchr      strchr
#define nc_strcmp      strcmp
#define nc_strcpy      strcpy
#define nc_strcspn     strcspn
#define nc_strlen      strlen
#define nc_strncat     strncat
#define nc_strncmp     strncmp
#define nc_strncpy     strncpy
#define nc_strpbrk     strpbrk
#define nc_strrchr     strrchr
#define nc_strspn      strspn
#define nc_strstr      strstr
#define nc_strtok      strtok
#define nc_isalnum     isalnum
#define nc_isalpha     isalpha
#define nc_iscntrl     iscntrl
#define nc_isdigit     isdigit
#define nc_isgraph     isgraph
#define nc_islower     islower
#define nc_isprint     isprint
#define nc_ispunct     ispunct
#define nc_isspace     isspace
#define nc_isupper     isupper
#define nc_isxdigit    isxdigit
#define nc_tolower     tolower
#define nc_toupper     toupper
#define nc_sprintf     sprintf
#define nc_atoi        atoi
#define nc_atol        atol
#define nc_strtod      strtod
#define nc_strtol      strtol
#define nc_strtoul     strtoul
#define nc_strlwr      _strlwr
#define nc_strupr      _strupr
#define nc_stricmp     _stricmp  ///perform a lowercase comparison of strings.
#define nc_strnicmp    _strnicmp ///compare characters of two strings without regard to case.

    ///////////////////////////////////////////////////////////
    ///global functions
#ifdef __cplusplus
    extern "C" {
#endif
//        void Sleep(DWORD sec);
//        pid_t gettid();
#ifdef __cplusplus
    }
#endif




    typedef UINT64 NIBtBDADDR;

}
#endif // NCTYPESDEFINE_H
/* EOF */
