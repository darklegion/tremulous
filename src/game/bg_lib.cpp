/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2013 Darklegion Development

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// bg_lib.c -- standard C library replacement routines used by code
// compiled for the virtual machine

#ifdef Q3_VM

#include "game/bg_lib.h"
#include "qcommon/q_shared.h"

/*-
 * Copyright (c) 1992, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "bg_lib.h"

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)qsort.c 8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] = "$Id$";
#endif /* LIBC_SCCS and not lint */

static char *med3(char *, char *, char *, cmp_t *);
static void swapfunc(char *, char *, int, int);

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n)      \
    {                                        \
        long i = (n) / sizeof(TYPE);         \
        register TYPE *pi = (TYPE *)(parmi); \
        register TYPE *pj = (TYPE *)(parmj); \
        do                                   \
        {                                    \
            register TYPE t = *pi;           \
            *pi++ = *pj;                     \
            *pj++ = t;                       \
        } while (--i > 0);                   \
    }

#define SWAPINIT(a, es) \
    swaptype = ((char *)a - (char *)0) % sizeof(long) || es % sizeof(long) ? 2 : es == sizeof(long) ? 0 : 1;

static void swapfunc(a, b, n, swaptype) char *a, *b;
int n, swaptype;
{
    if (swaptype <= 1)
        swapcode(long, a, b, n) else swapcode(char, a, b, n)
}

#define swap(a, b)                   \
    if (swaptype == 0)               \
    {                                \
        long t = *(long *)(a);       \
        *(long *)(a) = *(long *)(b); \
        *(long *)(b) = t;            \
    }                                \
    else                             \
        swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) \
    if ((n) > 0)         \
    swapfunc(a, b, n, swaptype)

static char *med3(a, b, c, cmp) char *a, *b, *c;
cmp_t *cmp;
{
    return cmp(a, b) < 0 ? (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a))
                         : (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c));
}

void qsort(a, n, es, cmp) void *a;
size_t n, es;
cmp_t *cmp;
{
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    int d, r, swaptype, swap_cnt;

loop:
    SWAPINIT(a, es);
    swap_cnt = 0;
    if (n < 7)
    {
        for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
            for (pl = pm; pl > (char *)a && cmp(pl - es, pl) > 0; pl -= es)
                swap(pl, pl - es);
        return;
    }
    pm = (char *)a + (n / 2) * es;
    if (n > 7)
    {
        pl = a;
        pn = (char *)a + (n - 1) * es;
        if (n > 40)
        {
            d = (n / 8) * es;
            pl = med3(pl, pl + d, pl + 2 * d, cmp);
            pm = med3(pm - d, pm, pm + d, cmp);
            pn = med3(pn - 2 * d, pn - d, pn, cmp);
        }
        pm = med3(pl, pm, pn, cmp);
    }
    swap(a, pm);
    pa = pb = (char *)a + es;

    pc = pd = (char *)a + (n - 1) * es;
    for (;;)
    {
        while (pb <= pc && (r = cmp(pb, a)) <= 0)
        {
            if (r == 0)
            {
                swap_cnt = 1;
                swap(pa, pb);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (r = cmp(pc, a)) >= 0)
        {
            if (r == 0)
            {
                swap_cnt = 1;
                swap(pc, pd);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc)
            break;
        swap(pb, pc);
        swap_cnt = 1;
        pb += es;
        pc -= es;
    }
    if (swap_cnt == 0)
    { /* Switch to insertion sort */
        for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
            for (pl = pm; pl > (char *)a && cmp(pl - es, pl) > 0; pl -= es)
                swap(pl, pl - es);
        return;
    }

    pn = (char *)a + n * es;
    r = MIN(pa - (char *)a, pb - pa);
    vecswap(a, pb - r, r);
    r = MIN(pd - pc, pn - pd - es);
    vecswap(pb, pn - r, r);
    if ((r = pb - pa) > es)
        qsort(a, r / es, es, cmp);
    if ((r = pd - pc) > es)
    {
        /* Iterate rather than recurse to save stack space */
        a = pn - r;
        n = r / es;
        goto loop;
    }
    /*    qsort(pn - r, r / es, es, cmp);*/
}

//==================================================================================

size_t strlen(const char *string)
{
    const char *s;

    s = string;
    while (*s)
        s++;

    return s - string;
}

char *strcat(char *strDestination, const char *strSource)
{
    char *s;

    s = strDestination;
    while (*s)
        s++;

    while (*strSource)
        *s++ = *strSource++;

    *s = 0;
    return strDestination;
}

char *strcpy(char *strDestination, const char *strSource)
{
    char *s;

    s = strDestination;

    while (*strSource)
        *s++ = *strSource++;

    *s = 0;
    return strDestination;
}

int strcmp(const char *string1, const char *string2)
{
    while (*string1 == *string2 && *string1 && *string2)
    {
        string1++;
        string2++;
    }

    return *string1 - *string2;
}

char *strrchr(const char *string, int c)
{
    int i, length = strlen(string);
    char *p;

    for (i = length /*sic*/; i >= 0; i--)
    {
        p = (char *)&string[i];

        if (*p == c)
            return (char *)p;
    }

    return (char *)0;
}

char *strchr(const char *string, int c)
{
    while (*string)
    {
        if (*string == c)
            return (char *)string;

        string++;
    }
    return c == '\0' ? (char *)string : (char *)0;
}

char *strstr(const char *string, const char *strCharSet)
{
    while (*string)
    {
        int i;

        for (i = 0; strCharSet[i]; i++)
        {
            if (string[i] != strCharSet[i])
                break;
        }

        if (!strCharSet[i])
            return (char *)string;

        string++;
    }
    return (char *)0;
}

int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c += 'a' - 'A';

    return c;
}

int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c += 'A' - 'a';

    return c;
}

void *memmove(void *dest, const void *src, size_t count)
{
    size_t i;

    if (dest > src)
    {
        i = count;
        while (i > 0)
        {
            i--;
            ((char *)dest)[i] = ((char *)src)[i];
        }
    }
    else
    {
        for (i = 0; i < count; i++)
            ((char *)dest)[i] = ((char *)src)[i];
    }

    return dest;
}

/*
===============
rint
===============
*/
double rint(double v)
{
    if (v >= 0.5f)
        return ceil(v);
    else
        return floor(v);
}

/*
===============
powN

Raise a double to a integer power
===============
*/
static double powN(double base, int exp)
{
    if (exp >= 0)
    {
        double result = 1.0;

        // calculate x, x^2, x^4, ... by repeated squaring
        // and multiply together the ones corresponding to the
        // binary digits of the exponent
        // e.g. x^73 = x^(1 + 8 + 64) = x * x^8 * x^64
        while (exp > 0)
        {
            if (exp % 2 == 1)
                result *= base;

            base *= base;
            exp /= 2;
        }

        return result;
    }
    // if exp is INT_MIN, the next clause will be upset,
    // because -exp isn't representable
    else if (exp == INT_MIN)
        return powN(base, exp + 1) / base;
    // x < 0
    else
        return 1.0 / powN(base, -exp);
}

double tan(double x) { return sin(x) / cos(x); }

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

typedef union {
    float value;
    unsigned int word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i, d)        \
    do                              \
    {                               \
        ieee_float_shape_type gf_u; \
        gf_u.value = (d);           \
        (i) = gf_u.word;            \
    } while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d, i)        \
    do                              \
    {                               \
        ieee_float_shape_type sf_u; \
        sf_u.word = (i);            \
        (d) = sf_u.value;           \
    } while (0)

/* A union which permits us to convert between a float and a 32 bit
   int.  */

// acos
static const float pi = 3.1415925026e+00, /* 0x40490fda */
    pio2_hi = 1.5707962513e+00, /* 0x3fc90fda */
    pio2_lo = 7.5497894159e-08, /* 0x33a22168 */
    pS0 = 1.6666667163e-01, /* 0x3e2aaaab */
    pS1 = -3.2556581497e-01, /* 0xbea6b090 */
    pS2 = 2.0121252537e-01, /* 0x3e4e0aa8 */
    pS3 = -4.0055535734e-02, /* 0xbd241146 */
    pS4 = 7.9153501429e-04, /* 0x3a4f7f04 */
    pS5 = 3.4793309169e-05, /* 0x3811ef08 */
    qS1 = -2.4033949375e+00, /* 0xc019d139 */
    qS2 = 2.0209457874e+00, /* 0x4001572d */
    qS3 = -6.8828397989e-01, /* 0xbf303361 */
    qS4 = 7.7038154006e-02; /* 0x3d9dc62e */

/*
==================
acos
==================
*/
double acos(double x)
{
    float z, subp, p, q, r, w, s, c, df;
    int hx, ix;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;

    if (ix == 0x3f800000)
    {  // |x|==1
        if (hx > 0)
            return 0.0;  // acos(1) = 0
        else
            return pi + (float)2.0 * pio2_lo;  // acos(-1)= pi
    }
    else if (ix > 0x3f800000)
    {  // |x| >= 1
        return (x - x) / (x - x);  // acos(|x|>1) is NaN
    }

    if (ix < 0x3f000000)
    {  // |x| < 0.5
        if (ix <= 0x23000000)
            return pio2_hi + pio2_lo;  // if|x|<2**-57

        z = x * x;
        subp = pS3 + z * (pS4 + z * pS5);
        // chop up expression to keep mac register based stack happy
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * subp)));
        q = 1.0 + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        r = p / q;
        return pio2_hi - (x - (pio2_lo - x * r));
    }
    else if (hx < 0)
    {  // x < -0.5
        z = (1.0 + x) * (float)0.5;
        subp = pS3 + z * (pS4 + z * pS5);
        // chop up expression to keep mac register based stack happy
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * subp)));
        q = 1.0 + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        s = sqrt(z);
        r = p / q;
        w = r * s - pio2_lo;
        return pi - (float)2.0 * (s + w);
    }
    else
    {  // x > 0.5
        int idf;
        z = (1.0 - x) * (float)0.5;
        s = sqrt(z);
        df = s;
        GET_FLOAT_WORD(idf, df);
        SET_FLOAT_WORD(df, idf & 0xfffff000);
        c = (z - df * df) / (s + df);
        subp = pS3 + z * (pS4 + z * pS5);
        // chop up expression to keep mac register based stack happy
        p = z * (pS0 + z * (pS1 + z * (pS2 + z * subp)));
        q = 1.0 + z * (qS1 + z * (qS2 + z * (qS3 + z * qS4)));
        r = p / q;
        w = r * s + c;
        return (double)(2.0 * (df + w));
    }
}

// pow
static const float bp[] =
    {
        1.0,
        1.5,
},
                   dp_h[] =
                       {
                           0.0,
                           5.84960938e-01,
}, /* 0x3f15c000 */
    dp_l[] =
        {
            0.0,
            1.56322085e-06,
}, /* 0x35d1cfdc */
    huge = 1.0e+30, tiny = 1.0e-30, zero = 0.0, one = 1.0, two = 2.0, two24 = 16777216.0, /* 0x4b800000 */
    two25 = 3.355443200e+07, /* 0x4c000000 */
    twom25 = 2.9802322388e-08, /* 0x33000000 */
    /* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
    L1 = 6.0000002384e-01, /* 0x3f19999a */
    L2 = 4.2857143283e-01, /* 0x3edb6db7 */
    L3 = 3.3333334327e-01, /* 0x3eaaaaab */
    L4 = 2.7272811532e-01, /* 0x3e8ba305 */
    L5 = 2.3066075146e-01, /* 0x3e6c3255 */
    L6 = 2.0697501302e-01, /* 0x3e53f142 */
    P1 = 1.6666667163e-01, /* 0x3e2aaaab */
    P2 = -2.7777778450e-03, /* 0xbb360b61 */
    P3 = 6.6137559770e-05, /* 0x388ab355 */
    P4 = -1.6533901999e-06, /* 0xb5ddea0e */
    P5 = 4.1381369442e-08, /* 0x3331bb4c */
    lg2 = 6.9314718246e-01, /* 0x3f317218 */
    lg2_h = 6.93145752e-01, /* 0x3f317200 */
    lg2_l = 1.42860654e-06, /* 0x35bfbe8c */
    ovt = 4.2995665694e-08, /* -(128-log2(ovfl+.5ulp)) */
    cp = 9.6179670095e-01, /* 0x3f76384f =2/(3ln2) */
    cp_h = 9.6179199219e-01, /* 0x3f763800 =head of cp */
    cp_l = 4.7017383622e-06, /* 0x369dc3a0 =tail of cp_h */
    ivln2 = 1.4426950216e+00, /* 0x3fb8aa3b =1/ln2 */
    ivln2_h = 1.4426879883e+00, /* 0x3fb8aa00 =16b 1/ln2*/
    ivln2_l = 7.0526075433e-06; /* 0x36eca570 =1/ln2 tail*/

/*
==================
copysignf
==================
*/
static float copysignf(float x, float y)
{
    unsigned int ix, iy;

    GET_FLOAT_WORD(ix, x);
    GET_FLOAT_WORD(iy, y);
    SET_FLOAT_WORD(x, (ix & 0x7fffffff) | (iy & 0x80000000));
    return x;
}

/*
==================
__scalbnf
==================
*/
static float __scalbnf(float x, int n)
{
    int k, ix;

    GET_FLOAT_WORD(ix, x);

    k = (ix & 0x7f800000) >> 23; /* extract exponent */

    if (k == 0)
    { /* 0 or subnormal x */
        if ((ix & 0x7fffffff) == 0)
            return x; /* +-0 */

        x *= two25;
        GET_FLOAT_WORD(ix, x);
        k = ((ix & 0x7f800000) >> 23) - 25;
    }
    if (k == 0xff)
        return x + x; /* NaN or Inf */

    k = k + n;

    if (n > 50000 || k > 0xfe)
        return huge * copysignf(huge, x); /* overflow  */
    if (n < -50000)
        return tiny * copysignf(tiny, x); /*underflow*/
    if (k > 0) /* normal result */
    {
        SET_FLOAT_WORD(x, (ix & 0x807fffff) | (k << 23));
        return x;
    }
    if (k <= -25)
        return tiny * copysignf(tiny, x); /*underflow*/

    k += 25; /* subnormal result */
    SET_FLOAT_WORD(x, (ix & 0x807fffff) | (k << 23));
    return x * twom25;
}

/*
==================
pow
==================
*/
float pow(float x, float y)
{
    float z, ax, z_h, z_l, p_h, p_l;
    float y1, subt1, t1, t2, subr, r, s, t, u, v, w;
    int i, j, k, yisint, n;
    int hx, hy, ix, iy, is;

    /*TA: for some reason the Q3 VM goes apeshit when x = 1.0
          and y > 1.0. Curiously this doesn't happen with gcc
          hence this hack*/
    if (x == 1.0)
        return x;

    GET_FLOAT_WORD(hx, x);
    GET_FLOAT_WORD(hy, y);
    ix = hx & 0x7fffffff;
    iy = hy & 0x7fffffff;

    /* y==zero: x**0 = 1 */
    if (iy == 0)
        return one;

    /* +-NaN return x+y */
    if (ix > 0x7f800000 || iy > 0x7f800000)
        return x + y;

    /* determine if y is an odd int when x < 0
     * yisint = 0 ... y is not an integer
     * yisint = 1 ... y is an odd int
     * yisint = 2 ... y is an even int
     */
    yisint = 0;
    if (hx < 0)
    {
        if (iy >= 0x4b800000)
            yisint = 2; /* even integer y */
        else if (iy >= 0x3f800000)
        {
            k = (iy >> 23) - 0x7f; /* exponent */
            j = iy >> (23 - k);
            if ((j << (23 - k)) == iy)
                yisint = 2 - (j & 1);
        }
    }

    /* special value of y */
    if (iy == 0x7f800000)
    { /* y is +-inf */
        if (ix == 0x3f800000)
            return y - y; /* inf**+-1 is NaN */
        else if (ix > 0x3f800000) /* (|x|>1)**+-inf = inf,0 */
            return (hy >= 0) ? y : zero;
        else /* (|x|<1)**-,+inf = inf,0 */
            return (hy < 0) ? -y : zero;
    }

    if (iy == 0x3f800000)
    { /* y is  +-1 */
        if (hy < 0)
            return one / x;
        else
            return x;
    }

    if (hy == 0x40000000)
        return x * x; /* y is  2 */

    if (hy == 0x3f000000)
    { /* y is  0.5 */
        if (hx >= 0) /* x >= +0 */
            return sqrt(x);
    }

    ax = fabs(x);

    /* special value of x */
    if (ix == 0x7f800000 || ix == 0 || ix == 0x3f800000)
    {
        z = ax; /*x is +-0,+-inf,+-1*/
        if (hy < 0)
            z = one / z; /* z = (1/|x|) */
        if (hx < 0)
        {
            if (((ix - 0x3f800000) | yisint) == 0)
                z = (z - z) / (z - z); /* (-1)**non-int is NaN */
            else if (yisint == 1)
                z = -z; /* (x<0)**odd = -(|x|**odd) */
        }

        return z;
    }

    /* (x<0)**(non-int) is NaN */
    if (((((unsigned int)hx >> 31) - 1) | yisint) == 0)
        return (x - x) / (x - x);

    /* |y| is huge */
    if (iy > 0x4d000000)
    { /* if |y| > 2**27 */
        /* over/underflow if x is not close to one */
        if (ix < 0x3f7ffff8)
            return (hy < 0) ? huge * huge : tiny * tiny;

        if (ix > 0x3f800007)
            return (hy > 0) ? huge * huge : tiny * tiny;
        /* now |1-x| is tiny <= 2**-20, suffice to compute
           log(x) by x-x^2/2+x^3/3-x^4/4 */
        t = x - 1; /* t has 20 trailing zeros */
        w = (t * t) * ((float)0.5 - t * ((float)0.333333333333 - t * (float)0.25));
        u = ivln2_h * t; /* ivln2_h has 16 sig. bits */
        v = t * ivln2_l - w * ivln2;
        t1 = u + v;
        GET_FLOAT_WORD(is, t1);
        SET_FLOAT_WORD(t1, is & 0xfffff000);
        t2 = v - (t1 - u);
    }
    else
    {
        float s2, s_h, s_l, t_h, t_l;
        n = 0;
        /* take care subnormal number */
        if (ix < 0x00800000)
        {
            ax *= two24;
            n -= 24;
            GET_FLOAT_WORD(ix, ax);
        }

        n += ((ix) >> 23) - 0x7f;
        j = ix & 0x007fffff;

        /* determine interval */
        ix = j | 0x3f800000; /* normalize ix */
        if (j <= 0x1cc471)
            k = 0; /* |x|<sqrt(3/2) */
        else if (j < 0x5db3d7)
            k = 1; /* |x|<sqrt(3)   */
        else
        {
            k = 0;
            n += 1;
            ix -= 0x00800000;
        }
        SET_FLOAT_WORD(ax, ix);

        /* compute s = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
        u = ax - bp[k]; /* bp[0]=1.0, bp[1]=1.5 */
        v = one / (ax + bp[k]);
        s = u * v;
        s_h = s;
        GET_FLOAT_WORD(is, s_h);
        SET_FLOAT_WORD(s_h, is & 0xfffff000);
        /* t_h=ax+bp[k] High */
        SET_FLOAT_WORD(t_h, ((ix >> 1) | 0x20000000) + 0x0040000 + (k << 21));
        t_l = ax - (t_h - bp[k]);
        s_l = v * ((u - s_h * t_h) - s_h * t_l);
        /* compute log(ax) */
        s2 = s * s;
        subr = L3 + s2 * (L4 + s2 * (L5 + s2 * L6));
        // chop up expression to keep mac register based stack happy
        r = s2 * s2 * (L1 + s2 * (L2 + s2 * subr));
        r += s_l * (s_h + s);
        s2 = s_h * s_h;
        t_h = (float)3.0 + s2 + r;
        GET_FLOAT_WORD(is, t_h);
        SET_FLOAT_WORD(t_h, is & 0xfffff000);
        t_l = r - ((t_h - (float)3.0) - s2);
        /* u+v = s*(1+...) */
        u = s_h * t_h;
        v = s_l * t_h + t_l * s;
        /* 2/(3log2)*(s+...) */
        p_h = u + v;
        GET_FLOAT_WORD(is, p_h);
        SET_FLOAT_WORD(p_h, is & 0xfffff000);
        p_l = v - (p_h - u);
        z_h = cp_h * p_h; /* cp_h+cp_l = 2/(3*log2) */
        z_l = cp_l * p_h + p_l * cp + dp_l[k];
        /* log2(ax) = (s+..)*2/(3*log2) = n + dp_h + z_h + z_l */
        t = (float)n;
        t1 = (((z_h + z_l) + dp_h[k]) + t);
        GET_FLOAT_WORD(is, t1);
        SET_FLOAT_WORD(t1, is & 0xfffff000);
        t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
    }

    s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
    if (((((unsigned int)hx >> 31) - 1) | (yisint - 1)) == 0)
        s = -one; /* (-ve)**(odd int) */

    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
    GET_FLOAT_WORD(is, y);
    SET_FLOAT_WORD(y1, is & 0xfffff000);
    p_l = (y - y1) * t1 + y * t2;
    p_h = y1 * t1;
    z = p_l + p_h;
    GET_FLOAT_WORD(j, z);

    if (j > 0x43000000) /* if z > 128 */
        return s * huge * huge; /* overflow */
    else if (j == 0x43000000)
    { /* if z == 128 */
        if (p_l + ovt > z - p_h)
            return s * huge * huge; /* overflow */
    }
    else if ((j & 0x7fffffff) > 0x43160000) /* z <= -150 */
        return s * tiny * tiny; /* underflow */
    else if ((unsigned int)j == 0xc3160000)
    { /* z == -150 */
        if (p_l <= z - p_h)
            return s * tiny * tiny; /* underflow */
    }

    /*
     * compute 2**(p_h+p_l)
     */
    i = j & 0x7fffffff;
    k = (i >> 23) - 0x7f;
    n = 0;

    if (i > 0x3f000000)
    { /* if |z| > 0.5, set n = [z+0.5] */
        n = j + (0x00800000 >> (k + 1));
        k = ((n & 0x7fffffff) >> 23) - 0x7f; /* new k for n */
        SET_FLOAT_WORD(t, n & ~(0x007fffff >> k));
        n = ((n & 0x007fffff) | 0x00800000) >> (23 - k);

        if (j < 0)
            n = -n;

        p_h -= t;
    }

    t = p_l + p_h;
    GET_FLOAT_WORD(is, t);
    SET_FLOAT_WORD(t, is & 0xfffff000);
    u = t * lg2_h;
    v = (p_l - (t - p_h)) * lg2 + t * lg2_l;
    z = u + v;
    w = v - (z - u);
    t = z * z;
    subt1 = P3 + t * (P4 + t * P5);
    // chop up expression to keep mac register based stack happy
    t1 = z - t * (P1 + t * (P2 + t * subt1));
    r = (z * t1) / (t1 - two) - (w + z * w);
    z = one - (r - z);
    GET_FLOAT_WORD(j, z);
    j += (n << 23);

    if ((j >> 23) <= 0)
        z = __scalbnf(z, n); /* subnormal output */
    else
        SET_FLOAT_WORD(z, j);

    return s * z;
}

static int randSeed = 0;

void srand(unsigned seed) { randSeed = seed; }

int rand(void)
{
    randSeed = (69069 * randSeed + 1);
    return randSeed & 0x7fff;
}

double atof(const char *string)
{
    float sign;
    float value;
    int c;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
            return 0;

        string++;
    }

    // check sign
    switch (*string)
    {
        case '+':
            string++;
            sign = 1;
            break;

        case '-':
            string++;
            sign = -1;
            break;

        default:
            sign = 1;
            break;
    }

    // read digits
    value = 0;
    c = string[0];

    if (c != '.')
    {
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
                break;

            c -= '0';
            value = value * 10 + c;
        } while (1);
    }
    else
        string++;

    // check for decimal point
    if (c == '.')
    {
        double fraction;

        fraction = 0.1;
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
                break;

            c -= '0';
            value += c * fraction;
            fraction *= 0.1;
        } while (1);
    }

    // not handling 10e10 notation...

    return value * sign;
}

/*
==============
strtod

Without an errno variable, this is a fair bit less useful than it is in libc
but it's still a fair bit more capable than atof or _atof
Handles inf[inity], nan (ignoring case), hexadecimals, and decimals
Handles decimal exponents like 10e10 and hex exponents like 0x7f8p20
10e10 == 10000000000 (power of ten)
0x7f8p20 == 0x7f800000 (decimal power of two)
The variable pointed to by endptr will hold the location of the first character
in the nptr string that was not used in the conversion
==============
*/
double strtod(const char *nptr, char **endptr)
{
    double res;
    bool neg = false;

    // skip whitespace
    while (isspace(*nptr))
        nptr++;

    // special string parsing
    if (Q_stricmpn(nptr, "nan", 3) == 0)
    {
        floatint_t nan;

        if (endptr)
            *endptr = (char *)&nptr[3];

        // nan can be followed by a bracketed number (in hex, octal,
        // or decimal) which is then put in the mantissa
        // this can be used to generate signalling or quiet NaNs, for
        // example (though I doubt it'll ever be used)
        // note that nan(0) is infinity!
        if (nptr[3] == '(')
        {
            char *end;
            int mantissa = strtol(&nptr[4], &end, 0);

            if (*end == ')')
            {
                nan.ui = 0x7f800000 | (mantissa & 0x7fffff);

                if (endptr)
                    *endptr = &end[1];
                return nan.f;
            }
        }

        nan.ui = 0x7fffffff;
        return nan.f;
    }

    if (Q_stricmpn(nptr, "inf", 3) == 0)
    {
        floatint_t inf;
        inf.ui = 0x7f800000;

        if (endptr == NULL)
            return inf.f;

        if (Q_stricmpn(&nptr[3], "inity", 5) == 0)
            *endptr = (char *)&nptr[8];
        else
            *endptr = (char *)&nptr[3];

        return inf.f;
    }

    // normal numeric parsing
    // sign
    if (*nptr == '-')
    {
        nptr++;
        neg = true;
    }
    else if (*nptr == '+')
        nptr++;

    // hex
    if (Q_stricmpn(nptr, "0x", 2) == 0)
    {
        // track if we use any digits
        const char *s = &nptr[1], *end = s;
        nptr += 2;

        for (res = 0;;)
        {
            if (isdigit(*nptr))
                res = 16 * res + (*nptr++ - '0');
            else if (*nptr >= 'A' && *nptr <= 'F')
                res = 16 * res + 10 + *nptr++ - 'A';
            else if (*nptr >= 'a' && *nptr <= 'f')
                res = 16 * res + 10 + *nptr++ - 'a';
            else
                break;
        }

        // if nptr moved, save it
        if (end + 1 < nptr)
            end = nptr;

        if (*nptr == '.')
        {
            float place;
            nptr++;

            // 1.0 / 16.0 == 0.0625
            // I don't expect the float accuracy to hold out for
            // very long but since we need to know the length of
            // the string anyway we keep on going regardless
            for (place = 0.0625;; place /= 16.0)
            {
                if (isdigit(*nptr))
                    res += place * (*nptr++ - '0');
                else if (*nptr >= 'A' && *nptr <= 'F')
                    res += place * (10 + *nptr++ - 'A');
                else if (*nptr >= 'a' && *nptr <= 'f')
                    res += place * (10 + *nptr++ - 'a');
                else
                    break;
            }

            if (end < nptr)
                end = nptr;
        }

        // parse an optional exponent, representing multiplication
        // by a power of two
        // exponents are only valid if we encountered at least one
        // digit already (and have therefore set end to something)
        if (end != s && tolower(*nptr) == 'p')
        {
            int exp;
            // apparently (confusingly) the exponent should be
            // decimal
            exp = strtol(&nptr[1], (char **)&end, 10);
            if (&nptr[1] == end)
            {
                // no exponent
                if (endptr)
                    *endptr = (char *)nptr;
                return res;
            }

            res *= powN(2, exp);
        }
        if (endptr)
            *endptr = (char *)end;
        return res;
    }
    // decimal
    else
    {
        // track if we find any digits
        const char *end = nptr, *p = nptr;
        // this is most of the work
        for (res = 0; isdigit(*nptr); res = 10 * res + *nptr++ - '0')
            ;
        // if nptr moved, we read something
        if (end < nptr)
            end = nptr;
        if (*nptr == '.')
        {
            // fractional part
            float place;
            nptr++;
            for (place = 0.1; isdigit(*nptr); place /= 10.0)
                res += (*nptr++ - '0') * place;
            // if nptr moved, we read something
            if (end + 1 < nptr)
                end = nptr;
        }
        // exponent
        // meaningless without having already read digits, so check
        // we've set end to something
        if (p != end && tolower(*nptr) == 'e')
        {
            int exp;
            exp = strtol(&nptr[1], (char **)&end, 10);
            if (&nptr[1] == end)
            {
                // no exponent
                if (endptr)
                    *endptr = (char *)nptr;
                return res;
            }

            res *= powN(10, exp);
        }
        if (endptr)
            *endptr = (char *)end;
        return res;
    }
}

double _atof(const char **stringPtr)
{
    const char *string;
    float sign;
    float value;
    int c = '0';

    string = *stringPtr;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
        {
            *stringPtr = string;
            return 0;
        }

        string++;
    }

    // check sign
    switch (*string)
    {
        case '+':
            string++;
            sign = 1;
            break;

        case '-':
            string++;
            sign = -1;
            break;

        default:
            sign = 1;
            break;
    }

    // read digits
    value = 0;
    if (string[0] != '.')
    {
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
                break;

            c -= '0';
            value = value * 10 + c;
        } while (1);
    }

    // check for decimal point
    if (c == '.')
    {
        double fraction;

        fraction = 0.1;
        do
        {
            c = *string++;
            if (c < '0' || c > '9')
                break;

            c -= '0';
            value += c * fraction;
            fraction *= 0.1;
        } while (1);
    }

    // not handling 10e10 notation...
    *stringPtr = string;

    return value * sign;
}

/*
==============
strtol

Handles any base from 2 to 36. If base is 0 then it guesses
decimal, hex, or octal based on the format of the number (leading 0 or 0x)
Will not overflow - returns LONG_MIN or LONG_MAX as appropriate
*endptr is set to the location of the first character not used
==============
*/
long strtol(const char *nptr, char **endptr, int base)
{
    long res;
    bool pos = true;

    if (endptr)
        *endptr = (char *)nptr;

    // bases other than 0, 2, 8, 16 are very rarely used, but they're
    // not much extra effort to support
    if (base < 0 || base == 1 || base > 36)
        return 0;

    // skip leading whitespace
    while (isspace(*nptr))
        nptr++;

    // sign
    if (*nptr == '-')
    {
        nptr++;
        pos = false;
    }
    else if (*nptr == '+')
        nptr++;

    // look for base-identifying sequences e.g. 0x for hex, 0 for octal
    if (nptr[0] == '0')
    {
        nptr++;

        // 0 is always a valid digit
        if (endptr)
            *endptr = (char *)nptr;

        if (*nptr == 'x' || *nptr == 'X')
        {
            if (base != 0 && base != 16)
            {
                // can't be hex, reject x (accept 0)
                if (endptr)
                    *endptr = (char *)nptr;
                return 0;
            }

            nptr++;
            base = 16;
        }
        else if (base == 0)
            base = 8;
    }
    else if (base == 0)
        base = 10;

    for (res = 0;;)
    {
        int val;

        if (isdigit(*nptr))
            val = *nptr - '0';
        else if (islower(*nptr))
            val = 10 + *nptr - 'a';
        else if (isupper(*nptr))
            val = 10 + *nptr - 'A';
        else
            break;

        if (val >= base)
            break;

        // we go negative because LONG_MIN is further from 0 than
        // LONG_MAX
        if (res < (LONG_MIN + val) / base)
            res = LONG_MIN;  // overflow
        else
            res = res * base - val;

        nptr++;

        if (endptr)
            *endptr = (char *)nptr;
    }
    if (pos)
    {
        // can't represent LONG_MIN positive
        if (res == LONG_MIN)
            res = LONG_MAX;
        else
            res = -res;
    }
    return res;
}

int atoi(const char *string)
{
    int sign;
    int value;
    int c;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
            return 0;

        string++;
    }

    // check sign
    switch (*string)
    {
        case '+':
            string++;
            sign = 1;
            break;

        case '-':
            string++;
            sign = -1;
            break;

        default:
            sign = 1;
            break;
    }

    // read digits
    value = 0;
    do
    {
        c = *string++;
        if (c < '0' || c > '9')
            break;

        c -= '0';
        value = value * 10 + c;
    } while (1);

    // not handling 10e10 notation...

    return value * sign;
}

int _atoi(const char **stringPtr)
{
    int sign;
    int value;
    int c;
    const char *string;

    string = *stringPtr;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
            return 0;

        string++;
    }

    // check sign
    switch (*string)
    {
        case '+':
            string++;
            sign = 1;
            break;

        case '-':
            string++;
            sign = -1;
            break;

        default:
            sign = 1;
            break;
    }

    // read digits
    value = 0;
    do
    {
        c = *string++;
        if (c < '0' || c > '9')
            break;

        c -= '0';
        value = value * 10 + c;
    } while (1);

    // not handling 10e10 notation...

    *stringPtr = string;

    return value * sign;
}

int abs(int n) { return n < 0 ? -n : n; }

double fabs(double x) { return x < 0 ? -x : x; }

unsigned int _hextoi(const char **stringPtr)
{
    unsigned int value;
    int c;
    int i;
    const char *string;

    string = *stringPtr;

    // skip whitespace
    while (*string <= ' ')
    {
        if (!*string)
            return 0;

        string++;
    }

    value = 0;
    i = 0;
    while (i++ < 8 && (c = *string++))
    {
        if (c >= '0' && c <= '9')
        {
            value = value * 16 + c - '0';
            continue;
        }
        else if (c >= 'a' && c <= 'f')
        {
            value = value * 16 + 10 + c - 'a';
            continue;
        }
        else if (c >= 'A' && c <= 'F')
        {
            value = value * 16 + 10 + c - 'A';
            continue;
        }
        else
            break;
    }
    *stringPtr = string;
    return value;
}

    //=========================================================

    /*
     * New implementation by Patrick Powell and others for vsnprintf.
     * Supports length checking in strings.
     */

    /*
     * Copyright Patrick Powell 1995
     * This code is based on code written by Patrick Powell (papowell@astart.com)
     * It may be used for any purpose as long as this notice remains intact
     * on all source code distributions
     */

    /**************************************************************
     * Original:
     * Patrick Powell Tue Apr 11 09:48:21 PDT 1995
     * A bombproof version of doprnt (dopr) included.
     * Sigh.  This sort of thing is always nasty do deal with.  Note that
     * the version here does not include floating point...
     *
     * snprintf() is used instead of sprintf() as it does limit checks
     * for string length.  This covers a nasty loophole.
     *
     * The other functions are there to prevent NULL pointers from
     * causing nast effects.
     *
     * More Recently:
     *  Brandon Long <blong@fiction.net> 9/15/96 for mutt 0.43
     *  This was ugly.  It is still ugly.  I opted out of floating point
     *  numbers, but the formatter understands just about everything
     *  from the normal C string format, at least as far as I can tell from
     *  the Solaris 2.5 printf(3S) man page.
     *
     *  Brandon Long <blong@fiction.net> 10/22/97 for mutt 0.87.1
     *    Ok, added some minimal floating point support, which means this
     *    probably requires libm on most operating systems.  Don't yet
     *    support the exponent (e,E) and sigfig (g,G).  Also, fmtint()
     *    was pretty badly broken, it just wasn't being exercised in ways
     *    which showed it, so that's been fixed.  Also, formated the code
     *    to mutt conventions, and removed dead code left over from the
     *    original.  Also, there is now a builtin-test, just compile with:
     *           gcc -DTEST_SNPRINTF -o snprintf snprintf.c -lm
     *    and run snprintf for results.
     *
     *  Thomas Roessler <roessler@guug.de> 01/27/98 for mutt 0.89i
     *    The PGP code was using unsigned hexadecimal formats.
     *    Unfortunately, unsigned formats simply didn't work.
     *
     *  Michael Elkins <me@cs.hmc.edu> 03/05/98 for mutt 0.90.8
     *    The original code assumed that both snprintf() and vsnprintf() were
     *    missing.  Some systems only have snprintf() but not vsnprintf(), so
     *    the code is now broken down under HAVE_SNPRINTF and HAVE_VSNPRINTF.
     *
     *  Andrew Tridgell (tridge@samba.org) Oct 1998
     *    fixed handling of %.0f
     *    added test for HAVE_LONG_DOUBLE
     *
     *  Russ Allbery <rra@stanford.edu> 2000-08-26
     *    fixed return value to comply with C99
     *    fixed handling of snprintf(NULL, ...)
     *
     *  Hrvoje Niksic <hniksic@arsdigita.com> 2000-11-04
     *    include <config.h> instead of "config.h".
     *    moved TEST_SNPRINTF stuff out of HAVE_SNPRINTF ifdef.
     *    include <stdio.h> for NULL.
     *    added support and test cases for long long.
     *    don't declare argument types to (v)snprintf if stdarg is not used.
     *    use int instead of short int as 2nd arg to va_arg.
     *
     **************************************************************/

    /* BDR 2002-01-13  %e and %g were being ignored.  Now do something,
       if not necessarily correctly */

#if (SIZEOF_LONG_DOUBLE > 0)
/* #ifdef HAVE_LONG_DOUBLE */
#define LDOUBLE long double
#else
#define LDOUBLE double
#endif

#if (SIZEOF_LONG_LONG > 0)
/* #ifdef HAVE_LONG_LONG */
#define LLONG long long
#else
#define LLONG long
#endif

static int dopr(char *buffer, size_t maxlen, const char *format, va_list args);
static int fmtstr(char *buffer, size_t *currlen, size_t maxlen, char *value, int flags, int min, int max);
static int fmtint(char *buffer, size_t *currlen, size_t maxlen, LLONG value, int base, int min, int max, int flags);
static int fmtfp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags);
static int dopr_outch(char *buffer, size_t *currlen, size_t maxlen, char c);

/*
 * dopr(): poor man's version of doprintf
 */

/* format read states */
#define DP_S_DEFAULT 0
#define DP_S_FLAGS 1
#define DP_S_MIN 2
#define DP_S_DOT 3
#define DP_S_MAX 4
#define DP_S_MOD 5
#define DP_S_MOD_L 6
#define DP_S_CONV 7
#define DP_S_DONE 8

/* format flags - Bits */
#define DP_F_MINUS (1 << 0)
#define DP_F_PLUS (1 << 1)
#define DP_F_SPACE (1 << 2)
#define DP_F_NUM (1 << 3)
#define DP_F_ZERO (1 << 4)
#define DP_F_UP (1 << 5)
#define DP_F_UNSIGNED (1 << 6)

/* Conversion Flags */
#define DP_C_SHORT 1
#define DP_C_LONG 2
#define DP_C_LLONG 3
#define DP_C_LDOUBLE 4

#define char_to_int(p) (p - '0')

static int dopr(char *buffer, size_t maxlen, const char *format, va_list args)
{
    char ch;
    LLONG value;
    LDOUBLE fvalue;
    char *strvalue;
    int min;
    int max;
    int state;
    int flags;
    int cflags;
    int total;
    size_t currlen;

    state = DP_S_DEFAULT;
    currlen = flags = cflags = min = 0;
    max = -1;
    ch = *format++;
    total = 0;

    while (state != DP_S_DONE)
    {
        if (ch == '\0')
            state = DP_S_DONE;

        switch (state)
        {
            case DP_S_DEFAULT:
                if (ch == '%')
                    state = DP_S_FLAGS;
                else
                    total += dopr_outch(buffer, &currlen, maxlen, ch);
                ch = *format++;
                break;
            case DP_S_FLAGS:
                switch (ch)
                {
                    case '-':
                        flags |= DP_F_MINUS;
                        ch = *format++;
                        break;
                    case '+':
                        flags |= DP_F_PLUS;
                        ch = *format++;
                        break;
                    case ' ':
                        flags |= DP_F_SPACE;
                        ch = *format++;
                        break;
                    case '#':
                        flags |= DP_F_NUM;
                        ch = *format++;
                        break;
                    case '0':
                        flags |= DP_F_ZERO;
                        ch = *format++;
                        break;
                    default:
                        state = DP_S_MIN;
                        break;
                }
                break;
            case DP_S_MIN:
                if ('0' <= ch && ch <= '9')
                {
                    min = 10 * min + char_to_int(ch);
                    ch = *format++;
                }
                else if (ch == '*')
                {
                    min = va_arg(args, int);
                    ch = *format++;
                    state = DP_S_DOT;
                }
                else
                    state = DP_S_DOT;
                break;
            case DP_S_DOT:
                if (ch == '.')
                {
                    state = DP_S_MAX;
                    ch = *format++;
                }
                else
                    state = DP_S_MOD;
                break;
            case DP_S_MAX:
                if ('0' <= ch && ch <= '9')
                {
                    if (max < 0)
                        max = 0;
                    max = 10 * max + char_to_int(ch);
                    ch = *format++;
                }
                else if (ch == '*')
                {
                    max = va_arg(args, int);
                    ch = *format++;
                    state = DP_S_MOD;
                }
                else
                    state = DP_S_MOD;
                break;
            case DP_S_MOD:
                switch (ch)
                {
                    case 'h':
                        cflags = DP_C_SHORT;
                        ch = *format++;
                        break;
                    case 'l':
                        cflags = DP_C_LONG;
                        ch = *format++;
                        break;
                    case 'L':
                        cflags = DP_C_LDOUBLE;
                        ch = *format++;
                        break;
                    default:
                        break;
                }
                if (cflags != DP_C_LONG)
                    state = DP_S_CONV;
                else
                    state = DP_S_MOD_L;
                break;
            case DP_S_MOD_L:
                switch (ch)
                {
                    case 'l':
                        cflags = DP_C_LLONG;
                        ch = *format++;
                        break;
                    default:
                        break;
                }
                state = DP_S_CONV;
                break;
            case DP_S_CONV:
                switch (ch)
                {
                    case 'd':
                    case 'i':
                        if (cflags == DP_C_SHORT)
                            value = (short int)va_arg(args, int);
                        else if (cflags == DP_C_LONG)
                            value = va_arg(args, long int);
                        else if (cflags == DP_C_LLONG)
                            value = va_arg(args, LLONG);
                        else
                            value = va_arg(args, int);
                        total += fmtint(buffer, &currlen, maxlen, value, 10, min, max, flags);
                        break;
                    case 'o':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT)
                            //    value = (unsigned short int) va_arg (args, unsigned short int); // Thilo: This does
                            //    not work because the rcc compiler cannot do that cast correctly.
                            value = va_arg(args, unsigned int) &
                                    ((1 << sizeof(unsigned short int) * 8) - 1);  // Using this workaround instead.
                        else if (cflags == DP_C_LONG)
                            value = va_arg(args, unsigned long int);
                        else if (cflags == DP_C_LLONG)
                            value = va_arg(args, unsigned LLONG);
                        else
                            value = va_arg(args, unsigned int);
                        total += fmtint(buffer, &currlen, maxlen, value, 8, min, max, flags);
                        break;
                    case 'u':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT)
                            value = va_arg(args, unsigned int) & ((1 << sizeof(unsigned short int) * 8) - 1);
                        else if (cflags == DP_C_LONG)
                            value = va_arg(args, unsigned long int);
                        else if (cflags == DP_C_LLONG)
                            value = va_arg(args, unsigned LLONG);
                        else
                            value = va_arg(args, unsigned int);
                        total += fmtint(buffer, &currlen, maxlen, value, 10, min, max, flags);
                        break;
                    case 'X':
                        flags |= DP_F_UP;
                    case 'x':
                        flags |= DP_F_UNSIGNED;
                        if (cflags == DP_C_SHORT)
                            value = va_arg(args, unsigned int) & ((1 << sizeof(unsigned short int) * 8) - 1);
                        else if (cflags == DP_C_LONG)
                            value = va_arg(args, unsigned long int);
                        else if (cflags == DP_C_LLONG)
                            value = va_arg(args, unsigned LLONG);
                        else
                            value = va_arg(args, unsigned int);
                        total += fmtint(buffer, &currlen, maxlen, value, 16, min, max, flags);
                        break;
                    case 'f':
                        if (cflags == DP_C_LDOUBLE)
                            fvalue = va_arg(args, LDOUBLE);
                        else
                            fvalue = va_arg(args, double);
                        /* um, floating point? */
                        total += fmtfp(buffer, &currlen, maxlen, fvalue, min, max, flags);
                        break;
                    case 'E':
                        flags |= DP_F_UP;
                    case 'e':
                        if (cflags == DP_C_LDOUBLE)
                            fvalue = va_arg(args, LDOUBLE);
                        else
                            fvalue = va_arg(args, double);
                        /* um, floating point? */
                        total += fmtfp(buffer, &currlen, maxlen, fvalue, min, max, flags);
                        break;
                    case 'G':
                        flags |= DP_F_UP;
                    case 'g':
                        if (cflags == DP_C_LDOUBLE)
                            fvalue = va_arg(args, LDOUBLE);
                        else
                            fvalue = va_arg(args, double);
                        /* um, floating point? */
                        total += fmtfp(buffer, &currlen, maxlen, fvalue, min, max, flags);
                        break;
                    case 'c':
                        total += dopr_outch(buffer, &currlen, maxlen, va_arg(args, int));
                        break;
                    case 's':
                        strvalue = va_arg(args, char *);
                        total += fmtstr(buffer, &currlen, maxlen, strvalue, flags, min, max);
                        break;
                    case 'p':
                        strvalue = va_arg(args, void *);
                        total += fmtint(buffer, &currlen, maxlen, (long)strvalue, 16, min, max, flags);
                        break;
                    case 'n':
                        if (cflags == DP_C_SHORT)
                        {
                            short int *num;
                            num = va_arg(args, short int *);
                            *num = currlen;
                        }
                        else if (cflags == DP_C_LONG)
                        {
                            long int *num;
                            num = va_arg(args, long int *);
                            *num = currlen;
                        }
                        else if (cflags == DP_C_LLONG)
                        {
                            LLONG *num;
                            num = va_arg(args, LLONG *);
                            *num = currlen;
                        }
                        else
                        {
                            int *num;
                            num = va_arg(args, int *);
                            *num = currlen;
                        }
                        break;
                    case '%':
                        total += dopr_outch(buffer, &currlen, maxlen, ch);
                        break;
                    case 'w':
                        /* not supported yet, treat as next char */
                        ch = *format++;
                        break;
                    default:
                        /* Unknown, skip */
                        break;
                }
                ch = *format++;
                state = DP_S_DEFAULT;
                flags = cflags = min = 0;
                max = -1;
                break;
            case DP_S_DONE:
                break;
            default:
                /* hmm? */
                break; /* some picky compilers need this */
        }
    }
    if (maxlen > 0)
        buffer[currlen] = '\0';
    return total;
}

static int fmtstr(char *buffer, size_t *currlen, size_t maxlen, char *value, int flags, int min, int max)
{
    int padlen, strln; /* amount to pad */
    int cnt = 0;
    int total = 0;

    if (value == 0)
    {
        value = "<NULL>";
    }

    for (strln = 0; value[strln]; ++strln)
        ; /* strlen */
    if (max >= 0 && max < strln)
        strln = max;
    padlen = min - strln;
    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justify */

    while (padlen > 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, ' ');
        --padlen;
    }
    while (*value && ((max < 0) || (cnt < max)))
    {
        total += dopr_outch(buffer, currlen, maxlen, *value++);
        ++cnt;
    }
    while (padlen < 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, ' ');
        ++padlen;
    }
    return total;
}

/* Have to handle DP_F_NUM (ie 0x and 0 alternates) */

static int fmtint(char *buffer, size_t *currlen, size_t maxlen, LLONG value, int base, int min, int max, int flags)
{
    int signvalue = 0;
    unsigned LLONG uvalue;
    char convert[24];
    int place = 0;
    int spadlen = 0; /* amount to space pad */
    int zpadlen = 0; /* amount to zero pad */
    const char *digits;
    int total = 0;

    if (max < 0)
        max = 0;

    uvalue = value;

    if (!(flags & DP_F_UNSIGNED))
    {
        if (value < 0)
        {
            signvalue = '-';
            uvalue = -value;
        }
        else if (flags & DP_F_PLUS) /* Do a sign (+/i) */
            signvalue = '+';
        else if (flags & DP_F_SPACE)
            signvalue = ' ';
    }

    if (flags & DP_F_UP)
        /* Should characters be upper case? */
        digits = "0123456789ABCDEF";
    else
        digits = "0123456789abcdef";

    do
    {
        convert[place++] = digits[uvalue % (unsigned)base];
        uvalue = (uvalue / (unsigned)base);
    } while (uvalue && (place < sizeof(convert)));
    if (place == sizeof(convert))
        place--;
    convert[place] = 0;

    zpadlen = max - place;
    spadlen = min - MAX(max, place) - (signvalue ? 1 : 0);
    if (zpadlen < 0)
        zpadlen = 0;
    if (spadlen < 0)
        spadlen = 0;
    if (flags & DP_F_ZERO)
    {
        zpadlen = MAX(zpadlen, spadlen);
        spadlen = 0;
    }
    if (flags & DP_F_MINUS)
        spadlen = -spadlen; /* Left Justifty */

#ifdef DEBUG_SNPRINTF
    dprint(1, (debugfile, "zpad: %d, spad: %d, min: %d, max: %d, place: %d\n", zpadlen, spadlen, min, max, place));
#endif

    /* Spaces */
    while (spadlen > 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, ' ');
        --spadlen;
    }

    /* Sign */
    if (signvalue)
        total += dopr_outch(buffer, currlen, maxlen, signvalue);

    /* Zeros */
    if (zpadlen > 0)
    {
        while (zpadlen > 0)
        {
            total += dopr_outch(buffer, currlen, maxlen, '0');
            --zpadlen;
        }
    }

    /* Digits */
    while (place > 0)
        total += dopr_outch(buffer, currlen, maxlen, convert[--place]);

    /* Left Justified spaces */
    while (spadlen < 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, ' ');
        ++spadlen;
    }

    return total;
}

static LDOUBLE abs_val(LDOUBLE value)
{
    LDOUBLE result = value;

    if (value < 0)
        result = -value;

    return result;
}

static long round(LDOUBLE value)
{
    long intpart;

    intpart = value;
    value = value - intpart;
    if (value >= 0.5)
        intpart++;

    return intpart;
}

static int fmtfp(char *buffer, size_t *currlen, size_t maxlen, LDOUBLE fvalue, int min, int max, int flags)
{
    int signvalue = 0;
    LDOUBLE ufvalue;
    char iconvert[20];
    char fconvert[20];
    int iplace = 0;
    int fplace = 0;
    int padlen = 0; /* amount to pad */
    int zpadlen = 0;
    int caps = 0;
    int total = 0;
    long intpart;
    long fracpart;

    /*
     * AIX manpage says the default is 0, but Solaris says the default
     * is 6, and sprintf on AIX defaults to 6
     */
    if (max < 0)
        max = 6;

    ufvalue = abs_val(fvalue);

    if (fvalue < 0)
        signvalue = '-';
    else if (flags & DP_F_PLUS) /* Do a sign (+/i) */
        signvalue = '+';
    else if (flags & DP_F_SPACE)
        signvalue = ' ';

#if 0
  if (flags & DP_F_UP) caps = 1; /* Should characters be upper case? */
#endif

    intpart = ufvalue;

    /*
     * Sorry, we only support 9 digits past the decimal because of our
     * conversion method
     */
    if (max > 9)
        max = 9;

    /* We "cheat" by converting the fractional part to integer by
     * multiplying by a factor of 10
     */
    fracpart = round((powN(10, max)) * (ufvalue - intpart));

    if (fracpart >= powN(10, max))
    {
        intpart++;
        fracpart -= powN(10, max);
    }

#ifdef DEBUG_SNPRINTF
    dprint(1, (debugfile, "fmtfp: %f =? %d.%d\n", fvalue, intpart, fracpart));
#endif

    /* Convert integer part */
    do
    {
        iconvert[iplace++] = (caps ? "0123456789ABCDEF" : "0123456789abcdef")[intpart % 10];
        intpart = (intpart / 10);
    } while (intpart && (iplace < 20));
    if (iplace == 20)
        iplace--;
    iconvert[iplace] = 0;

    /* Convert fractional part */
    do
    {
        fconvert[fplace++] = (caps ? "0123456789ABCDEF" : "0123456789abcdef")[fracpart % 10];
        fracpart = (fracpart / 10);
    } while (fracpart && (fplace < 20));
    if (fplace == 20)
        fplace--;
    fconvert[fplace] = 0;

    /* -1 for decimal point, another -1 if we are printing a sign */
    padlen = min - iplace - max - 1 - ((signvalue) ? 1 : 0);
    zpadlen = max - fplace;
    if (zpadlen < 0)
        zpadlen = 0;
    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justifty */

    if ((flags & DP_F_ZERO) && (padlen > 0))
    {
        if (signvalue)
        {
            total += dopr_outch(buffer, currlen, maxlen, signvalue);
            --padlen;
            signvalue = 0;
        }
        while (padlen > 0)
        {
            total += dopr_outch(buffer, currlen, maxlen, '0');
            --padlen;
        }
    }
    while (padlen > 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, ' ');
        --padlen;
    }
    if (signvalue)
        total += dopr_outch(buffer, currlen, maxlen, signvalue);

    while (iplace > 0)
        total += dopr_outch(buffer, currlen, maxlen, iconvert[--iplace]);

    /*
     * Decimal point.  This should probably use locale to find the correct
     * char to print out.
     */
    if (max > 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, '.');

        while (zpadlen-- > 0)
            total += dopr_outch(buffer, currlen, maxlen, '0');

        while (fplace > 0)
            total += dopr_outch(buffer, currlen, maxlen, fconvert[--fplace]);
    }

    while (padlen < 0)
    {
        total += dopr_outch(buffer, currlen, maxlen, ' ');
        ++padlen;
    }

    return total;
}

static int dopr_outch(char *buffer, size_t *currlen, size_t maxlen, char c)
{
    if (*currlen + 1 < maxlen)
        buffer[(*currlen)++] = c;
    return 1;
}

int Q_vsnprintf(char *str, size_t length, const char *fmt, va_list args) { return dopr(str, length, fmt, args); }

int Q_snprintf(char *str, size_t length, const char *fmt, ...)
{
    va_list ap;
    int retval;

    va_start(ap, fmt);
    retval = Q_vsnprintf(str, length, fmt, ap);
    va_end(ap);

    return retval;
}

/* this is really crappy */
int sscanf(const char *buffer, const char *fmt, ...)
{
    int cmd;
    va_list ap;
    int count;
    size_t len;

    va_start(ap, fmt);
    count = 0;

    while (*fmt)
    {
        if (fmt[0] != '%')
        {
            fmt++;
            continue;
        }

        fmt++;
        cmd = *fmt;

        if (isdigit(cmd))
        {
            len = (size_t)_atoi(&fmt);
            cmd = *(fmt - 1);
        }
        else
        {
            len = MAX_STRING_CHARS - 1;
            fmt++;
        }

        switch (cmd)
        {
            case 'i':
            case 'd':
            case 'u':
                *(va_arg(ap, int *)) = _atoi(&buffer);
                break;
            case 'f':
                *(va_arg(ap, float *)) = _atof(&buffer);
                break;
            case 'x':
                *(va_arg(ap, unsigned int *)) = _hextoi(&buffer);
                break;
            case 's':
            {
                char *s = va_arg(ap, char *);
                while (isspace(*buffer))
                    buffer++;
                while (*buffer && !isspace(*buffer) && len-- > 0)
                    *s++ = *buffer++;
                *s++ = '\0';
                break;
            }
        }
    }

    va_end(ap);
    return count;
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, cmp_t *compar)
{
    size_t low = 0, high = nmemb, mid;
    int comp;
    void *ptr;

    while (low < high)
    {
        mid = low + (high - low) / 2;
        ptr = (void *)((char *)base + (mid * size));
        comp = compar(key, ptr);
        if (comp < 0)
            high = mid;
        else if (comp > 0)
            low = mid + 1;
        else
            return ptr;
    }
    return NULL;
}

#endif
