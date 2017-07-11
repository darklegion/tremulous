/****************************************************************************
 *
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2009-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "strvec.h"

typedef struct {
    char** v;
    unsigned n;
} StringVector;

void* StringVector_New (void)
{
    StringVector* sv = malloc(sizeof(*sv));
    if ( !sv )
        return NULL;

    sv->v = malloc(sizeof(*sv->v));
    if ( !sv->v )
    {
        free(sv);
        return NULL;
    }

    sv->n = 0;
    return sv;
}

void StringVector_Delete (void* pv)
{
    unsigned i;
    StringVector* sv = (StringVector*)pv;

    if ( !sv )
        return;

    for ( i = 0; i < sv->n; i++ )
        free(sv->v[i]);

    free(sv->v);
    free(sv);
}

int StringVector_Add (void* pv, const char* s)
{
    StringVector* sv = (StringVector*)pv;
    char** v;
    char* temp;

    if ( !sv || !s )
        return 0;

    temp = strdup(s);
    if ( !temp )
        return 0;

    v = realloc(sv->v, (sv->n+2) * sizeof(char*));
    if ( !v )
        return 0;

    sv->v = v;
    sv->v[sv->n++] = temp; 
    sv->v[sv->n] = NULL;

    return 1;
}

char* StringVector_Get (void* pv, unsigned index)
{
    StringVector* sv = (StringVector*)pv;

    if ( !sv || index >= sv->n )
        return NULL;

    return sv->v[index];
}

int StringVector_AddVector (void* pd, void* ps)
{
    unsigned i = 0;
    const char* s = StringVector_Get(ps, i++);

    while ( s )
    {
        if ( !StringVector_Add(pd, s) )
            return 0;

        s = StringVector_Get(ps, i++);
    }
    return 1;
}

const char** StringVector_GetVector (void* pv)
{
    StringVector* sv = (StringVector*)pv;

    if ( !sv )
        return NULL;

    return (const char**)sv->v;
}

const char* StringVector_toString(void* pv)
{
    unsigned i = 0;
    const char* s = StringVector_Get(pv, i++);

    char* ret = NULL;
    size_t siz = 0;

    while ( s )
    {
        size_t n = siz + strlen(s) + 2;
        size_t o = n - 1;

        char* _tmp = realloc(ret, n);
        if ( !_tmp )
        {
            free(ret);
            return NULL;
        }
        ret = _tmp;

        memcpy(ret + siz, s, strlen(s));
        ret[o] = ' ';
        siz = n;

        s = StringVector_Get(pv, i++);
    }

    if ( ret && siz )
        ret[siz-1] = '\0';

    return ret;
}

