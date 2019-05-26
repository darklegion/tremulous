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

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include "nettle/bignum.h"
#include "nettle/buffer.h"
#include "nettle/rsa.h"
#include "nettle/sha2.h"


#define RSA_PRIVATE_KEY_FILE "rsa_private_key"
#define RSA_PUBLIC_KEY_FILE "rsa_public_key"

#define RSA_PUBLIC_EXPONENT 65537

#define RSA_KEY_LENGTH 4096
#define RSA_STRING_LENGTH (RSA_KEY_LENGTH / 4 + 1)

void Crypto_Init( void );
void qnettle_random( void *ctx, size_t length, uint8_t *dst );

#endif /* __CRYPTO_H__ */
