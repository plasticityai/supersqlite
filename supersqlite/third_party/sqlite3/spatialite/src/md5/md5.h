/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#elif !defined(_MD5_H)
#define _MD5_H

/*

 SpatiaLite private MD5 declarations

 Sandro Furieri - 2013-06-03
*/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#ifdef _WIN32
#ifdef DLL_EXPORT
#define MD5_PRIVATE
#else
#define MD5_PRIVATE
#endif
#else
#define MD5_PRIVATE __attribute__ ((visibility("hidden")))
#endif
#endif
/* end Sandro Furieri - SpatiaLite - 2013-06-03 */

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;

MD5_PRIVATE void splite_MD5_Init(MD5_CTX *ctx);
MD5_PRIVATE void splite_MD5_Update(MD5_CTX *ctx, void *data, unsigned long size);
MD5_PRIVATE void splite_MD5_Final(unsigned char *result, MD5_CTX *ctx);

#endif
