/*
===========================================================================
Copyright (C) 2007-2008 Amanieu d'Antras (amanieu@gmail.com)
Copyright (C) 2015-2016 Jeff Kent (jeff@jkent.net)
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

#include "crypto.h"

#include "sys/sys_shared.h"

#include "cvar.h"
#include "q_shared.h"
#include "qcommon.h"

#define TO_REAL_PTR(x) ((uint8_t*)x - sizeof(size_t))
#define TO_MOCK_PTR(x) ((uint8_t*)x + sizeof(size_t))
#define REAL_PTR_SIZE(x) (*((size_t *)x))
#define MOCK_PTR_SIZE(x) (REAL_PTR_SIZE(TO_REAL_PTR(x)))


static void *crypto_alloc( size_t size )
{
	void *p;

	assert( size > 0 );

	p = malloc( sizeof(size_t) + size );
	if ( !p )
		Com_Error( ERR_FATAL, "crypto_alloc: Virtual memory exhausted." );

	REAL_PTR_SIZE( p ) = size;
	return TO_MOCK_PTR( p );
}

static void *crypto_realloc( void *old, size_t old_size, size_t new_size )
{
	void *p;

	old_size = MOCK_PTR_SIZE( old );
	if ( new_size == old_size ) {
		return old;
	}

	p = malloc( sizeof(size_t) + new_size );
	if ( !p )
		Com_Error( ERR_FATAL, "crypto_realloc: Virtual memory exhausted." );
	REAL_PTR_SIZE( p ) = new_size;

	p = TO_MOCK_PTR( p );
	memcpy( p, old, MIN( old_size, new_size ) );
	old = TO_REAL_PTR( old );
	memset( old, 0, sizeof(size_t) + old_size );
	free( old );

	return p;
}

static void crypto_free( void *p, size_t size )
{
	p = TO_REAL_PTR( p );
	size = REAL_PTR_SIZE( p );
	memset( p, 0, sizeof(size_t) + size );
	free( p );
}

void Crypto_Init( void )
{
	mp_set_memory_functions( crypto_alloc, crypto_realloc, crypto_free );
}

void qnettle_random( void *ctx, size_t length, uint8_t *dst )
{
	Sys_CryptoRandomBytes( dst, length );
}
