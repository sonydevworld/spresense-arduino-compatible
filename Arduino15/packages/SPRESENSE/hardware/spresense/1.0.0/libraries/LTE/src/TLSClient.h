/*
 *  TLSClient.h - TLSClient include file for Spresense Arduino
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _TLS_CLIENT_H_
#define _TLS_CLIENT_H_

#ifdef SUBCORE
#error "TLSClient library is NOT supported by SubCore."
#endif

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <mbedtls/config.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/platform.h>
#include <mbedtls/ssl.h>

typedef struct tlsClientContext_s
{
  mbedtls_ssl_context      ssl;
  mbedtls_ssl_config       conf;
  mbedtls_net_context      serverFd;
  mbedtls_ctr_drbg_context ctrDrbg;
  mbedtls_entropy_context  entropy;
  mbedtls_x509_crt         caCert;
  mbedtls_x509_crt         cliCert;
  mbedtls_pk_context       cliKey;
} tlsClientContext_t;

void tlsInit(tlsClientContext_t *tlsCtx);
void tlsShutdown(tlsClientContext_t *tlsCtx);
int tlsConnect(tlsClientContext_t *tlsCtx, const char *host, uint32_t port,
               uint32_t timeout,
               const char *rootCA, size_t rootCASize, 
               const char *clientCA,  size_t clientCASize, 
               const char *privateKey,  size_t privateKeySize);
int tlsGetAvailable(tlsClientContext_t *tlsCtx);
int tlsRead(tlsClientContext_t *tlsCtx, uint8_t *buffer, int len);
int tlsWrite(tlsClientContext_t *tlsCtx, const uint8_t *buffer, int len,
             uint32_t timeout);

#endif
