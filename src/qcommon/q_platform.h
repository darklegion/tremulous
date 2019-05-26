/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development
Copyright (C) 2015-2019 GrangerHub

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/

#ifndef __Q_PLATFORM_H
#define __Q_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

// this is for determining if we have an asm version of a C function
#define idx64 0

#ifdef Q3_VM

#define id386 0
#define idppc 0
#define idppc_altivec 0
#define idsparc 0

#else

#if (defined _M_IX86 || defined __i386__) && !defined(C_ONLY)
#define id386 1
#else
#define id386 0
#endif

#if (defined(powerc) || defined(powerpc) || defined(ppc) || \
	defined(__ppc) || defined(__ppc__)) && !defined(C_ONLY)
#define idppc 1
#if defined(__VEC__)
#define idppc_altivec 1
#ifdef __APPLE__  // Apple's GCC does this differently than the FSF.
#define VECCONST_UINT8(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
	(vector unsigned char) (a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
#else
#define VECCONST_UINT8(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
	(vector unsigned char) {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p}
#endif
#else
#define idppc_altivec 0
#endif
#else
#define idppc 0
#define idppc_altivec 0
#endif

#if defined(__sparc__) && !defined(C_ONLY)
#define idsparc 1
#else
#define idsparc 0
#endif

#endif

#ifndef __ASM_I386__ // don't include the C bits if included from qasm.h

// for windows fastcall option
#define QDECL
#define QCALL

//================================================================= WIN64/32 ===

#if defined(_WIN64) || defined(__WIN64__)

    #undef idx64
    #define idx64 1
    
    #undef QDECL
    #define QDECL __cdecl
    
    #undef QCALL
    #define QCALL __stdcall
    
    #if defined( _MSC_VER )
    #define OS_STRING "win_msvc64"
    #elif defined __MINGW64__
    #define OS_STRING "win_mingw64"
    #endif
    
    #define ID_INLINE __inline
    #define PATH_SEP '\\'
    
    #if defined( __WIN64__ ) 
    #define ARCH_STRING "x86_64"
    #elif defined _M_ALPHA
    #define ARCH_STRING "AXP"
    #endif
    
    #define Q3_LITTLE_ENDIAN
    
    #define DLL_EXT ".dll"
    #define EXE_EXT ".exe"

    // For cl_updates.cpp
    #define RELEASE_PACKAGE_NAME ( "release-mingw32-" ARCH_STRING ".zip" )
    #define RELEASE_SIGNATURE_NAME ( "release-mingw32-" ARCH_STRING ".zip.sig" )
    #define GRANGER_EXE ( "granger" EXE_EXT )

#elif defined(_WIN32) || defined(__WIN32__)

    #undef QDECL
    #define QDECL __cdecl
    
    #undef QCALL
    #define QCALL __stdcall
    
    #if defined( _MSC_VER )
    #define OS_STRING "win_msvc"
    #elif defined __MINGW32__
    #define OS_STRING "win_mingw"
    #endif
    
    #define ID_INLINE __inline
    #define PATH_SEP '\\'
    
    #if defined( _M_IX86 ) || defined( __i386__ )
    #define ARCH_STRING "x86"
    #elif defined _M_ALPHA
    #define ARCH_STRING "AXP"
    #endif
    
    #define Q3_LITTLE_ENDIAN
    
    #define DLL_EXT ".dll"
    #define EXE_EXT ".exe"

    // For cl_updates.cpp
    #define RELEASE_PACKAGE_NAME ( "release-mingw32-" ARCH_STRING ".zip" )
    #define RELEASE_SIGNATURE_NAME ( "release-mingw32-" ARCH_STRING ".zip.sig" )
    #define GRANGER_EXE ( "granger" EXE_EXT )

#endif


//============================================================== MAC OS X ===

#if defined(__APPLE__) || defined(__APPLE_CC__)

    #define OS_STRING "macosx"
    #define ID_INLINE inline
    #define PATH_SEP '/'

    #ifdef __ppc__
    #define ARCH_STRING "ppc"
    #define Q3_BIG_ENDIAN
    #elif defined __i386__
    #define ARCH_STRING "x86"
    #define Q3_LITTLE_ENDIAN
    #elif defined __x86_64__
    #undef idx64
    #define idx64 1
    #define ARCH_STRING "x86_64"
    #define Q3_LITTLE_ENDIAN
    #endif

    #define DLL_EXT ".dylib"
    #define EXE_EXT

    // For cl_updates.cpp
    #define RELEASE_PACKAGE_NAME ( "release-darwin-" ARCH_STRING ".zip" )
    #define RELEASE_SIGNATURE_NAME ( "release-darwin-" ARCH_STRING ".zip.sig" )
    #define GRANGER_EXE ( "granger" EXE_EXT )

#endif

//================================================================= LINUX ===


#if defined(__linux__) || defined(__FreeBSD_kernel__) || defined(__GNU__)
    
    #include <endian.h>
    
    #if defined(__linux__)
    #       define OS_STRING "linux"
    #elif defined(__FreeBSD_kernel__)
    #       define OS_STRING "kFreeBSD"
    #else
    #       define OS_STRING "GNU"
    #endif
    
    #define ID_INLINE inline
    #define PATH_SEP '/'
    
    #ifdef __ppc__
     #define ARCH_STRING "ppc"
     #define Q3_BIG_ENDIAN
    #elif defined __i386__
     # define ARCH_STRING "x86"
     # define Q3_LITTLE_ENDIAN
    #elif defined __x86_64__
     # define ARCH_STRING "x86_64"
     # define Q3_LITTLE_ENDIAN
     #undef idx64
     #define idx64 1
    #elif defined __arm__
     # if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      # error "Big endian ARM is not supported"
     # endif
     # if defined __armhf__
      # define ARCH_STRING "armhf"
      # define Q3_LITTLE_ENDIAN
     # else
      # define ARCH_STRING "armel"
      # define Q3_LITTLE_ENDIAN
     # endif
    #elif defined __aarch64__
     # if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      # error "Big endian ARM is not supported"
     # endif
     # define ARCH_STRING "aarch64"
     # define Q3_LITTLE_ENDIAN
    #endif
    
    #define DLL_EXT ".so"
    #define EXE_EXT
    
    // For cl_updates.cpp
    #define RELEASE_PACKAGE_NAME ( "release-linux-" ARCH_STRING ".zip" )
    #define RELEASE_SIGNATURE_NAME ( "release-linux-" ARCH_STRING ".zip.sig" )
    #define GRANGER_EXE ( "granger" EXE_EXT )
 
#endif

//=================================================================== BSD ===

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

    #include <sys/types.h>
    #include <machine/endian.h>

    #ifndef __BSD__
      #define __BSD__
    #endif

    #if defined(__FreeBSD__)
    #define OS_STRING "freebsd"
    #elif defined(__OpenBSD__)
    #define OS_STRING "openbsd"
    #elif defined(__NetBSD__)
    #define OS_STRING "netbsd"
    #endif

    #define ID_INLINE inline
    #define PATH_SEP '/'

    #ifdef __i386__
    #define ARCH_STRING "x86"
    #elif defined __amd64__
    #undef idx64
    #define idx64 1
    #define ARCH_STRING "x86_64"
    #elif defined __axp__
    #define ARCH_STRING "alpha"
    #endif

    #if BYTE_ORDER == BIG_ENDIAN
    #define Q3_BIG_ENDIAN
    #else
    #define Q3_LITTLE_ENDIAN
    #endif

    #define DLL_EXT ".so"
    #define EXE_EXT

#endif

//================================================================= SUNOS ===

#ifdef __sun

    #include <stdint.h>
    #include <sys/byteorder.h>

    #define OS_STRING "solaris"
    #define ID_INLINE inline
    #define PATH_SEP '/'

    #ifdef __i386__
    #define ARCH_STRING "x86"
    #elif defined __sparc
    #define ARCH_STRING "sparc"
    #endif

    #if defined( _BIG_ENDIAN )
    #define Q3_BIG_ENDIAN
    #elif defined( _LITTLE_ENDIAN )
    #define Q3_LITTLE_ENDIAN
    #endif

    #define DLL_EXT ".so"
    #define EXE_EXT

#endif

//================================================================== IRIX ===

#ifdef __sgi

    #define OS_STRING "irix"
    #define ID_INLINE __inline
    #define PATH_SEP '/'

    #define ARCH_STRING "mips"

    #define Q3_BIG_ENDIAN // SGI's MIPS are always big endian

    #define DLL_EXT ".so"
    #define EXE_EXT

#endif

//================================================================== Q3VM ===

#ifdef Q3_VM

    #define OS_STRING "q3vm"
    #define ID_INLINE
    #define PATH_SEP '/'

    #define ARCH_STRING "bytecode"

    #define DLL_EXT ".qvm"
    #define EXE_EXT

#endif

//===========================================================================

//catch missing defines in above blocks
#if !defined( OS_STRING )
#error "Operating system not supported"
#endif

#if !defined( ARCH_STRING )
#error "Architecture not supported"
#endif

#ifndef ID_INLINE
#error "ID_INLINE not defined"
#endif

#ifndef PATH_SEP
#error "PATH_SEP not defined"
#endif

#ifndef DLL_EXT
#error "DLL_EXT not defined"
#endif

#ifndef EXE_EXT
#error "EXE_EXT not defined"
#endif


//endianness
void CopyShortSwap (void *dest, void *src);
void CopyLongSwap (void *dest, void *src);
short ShortSwap (short l);
int LongSwap (int l);
float FloatSwap (const float *f);

#if defined( Q3_BIG_ENDIAN ) && defined( Q3_LITTLE_ENDIAN )
 #error "Endianness defined as both big and little"
#elif defined( Q3_BIG_ENDIAN )

 #define CopyLittleShort(dest, src) CopyShortSwap(dest, src)
 #define CopyLittleLong(dest, src) CopyLongSwap(dest, src)
 #define LittleShort(x) ShortSwap(x)
 #define LittleLong(x) LongSwap(x)
 #define LittleFloat(x) FloatSwap(&x)
 #define BigShort
 #define BigLong
 #define BigFloat

#elif defined( Q3_LITTLE_ENDIAN )

 #define CopyLittleShort(dest, src) memcpy(dest, src, 2)
 #define CopyLittleLong(dest, src) memcpy(dest, src, 4)
 #define LittleShort
 #define LittleLong
 #define LittleFloat
 #define BigShort(x) ShortSwap(x)
 #define BigLong(x) LongSwap(x)
 #define BigFloat(x) FloatSwap(&x)

#elif defined( Q3_VM )

 #define LittleShort
 #define LittleLong
 #define LittleFloat
 #define BigShort
 #define BigLong
 #define BigFloat

#else

 #error "Endianness not defined"

#endif


//platform string
#ifdef NDEBUG
 #define PLATFORM_STRING OS_STRING "-" ARCH_STRING
#else
 #define PLATFORM_STRING OS_STRING "-" ARCH_STRING "-debug"
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
