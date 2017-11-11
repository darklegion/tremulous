/* Copyright (c) 2010-2012, Victor J. Roemer. All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __CDEFS_H__
#define __CDEFS_H__

#define UNUSED __attribute__((unused))

#define NORETURN __attribute__((noreturn))

/* Support for flexible arrays, stolen from dnet */
#undef __flexarr
#if defined(__GNUC__) && ((__GNUC__ > 2) || \
    (__GNUC__ == 2 && __GNUC_MINOR__ >= 97))

/* GCC 2.97 supports C99 flexible array members.  */
# define __flexarr	[]
#else
# ifdef __GNUC__
#  define __flexarr	[0]
# else
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#   define __flexarr	[]
#  elif defined(_WIN32)
/* MS VC++ */
#   define __flexarr	[]
#  else
/* Some other non-C99 compiler. Approximate with [1]. */
#   define __flexarr	[1]
#  endif
# endif
#endif

#ifndef SO_PUBLIC
#if defined _WIN32 || defined __CYGWIN__
#  ifdef __GNUC__
#    define SO_PUBLIC __attribute__((dllimport))
#  else
#    define SO_PUBLIC __declspec(dllimport)
#  endif
#  define DLL_LOCAL
#else
#  ifdef HAVE_VISIBILITY
#    define SO_PUBLIC  __attribute__ ((visibility("default")))
#    define SO_PRIVATE __attribute__ ((visibility("hidden")))
#  else
#    define SO_PUBLIC
#    define SO_PRIVATE
#  endif
#endif
#endif

#endif
