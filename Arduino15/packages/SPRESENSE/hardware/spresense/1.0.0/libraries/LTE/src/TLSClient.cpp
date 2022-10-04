/*
 *  TLSClient.cpp - TLSClient implementation file for Spresense Arduino
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <string.h>
#include <TLSClient.h>
#include <WString.h>
#include <time.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef BRD_DEBUG
#define TLSCDBG(format, ...) ::printf("DEBUG: " format, ##__VA_ARGS__)
#else
#define TLSCDBG(format, ...)
#endif
#define TLSCERR(format, ...) ::printf("ERROR: " format, ##__VA_ARGS__)

#define BUF_LEN 512

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char *g_pers = "spresense-tls";

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void startTimer(struct timespec *timer)
{
  clock_gettime(CLOCK_MONOTONIC, timer);
}

static uint32_t leftTimer(struct timespec *timer, uint32_t timeout_ms)
{
  struct timespec current_time;
  struct timespec diff_time;
  uint32_t difftime_msec = 0;

  clock_gettime(CLOCK_MONOTONIC, &current_time);

  diff_time.tv_sec  = current_time.tv_sec - timer->tv_sec;
  diff_time.tv_nsec = current_time.tv_nsec - timer->tv_nsec;

  if (diff_time.tv_nsec < 0) {
    diff_time.tv_sec -= 1;
    diff_time.tv_nsec += 1000*1000*1000;
  }

  difftime_msec = diff_time.tv_sec*1000 + diff_time.tv_nsec/(1000*1000);

  if (difftime_msec < timeout_ms) {
    return timeout_ms - difftime_msec;
  }

  return 0;
}

static bool hasTimerExpired(struct timespec *timer, uint32_t timeout_ms)
{
  if (timeout_ms) {
    return leftTimer(timer, timeout_ms) ? false : true;
  }

  /* 0 means no timeout */

  return false;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void tlsInit(tlsClientContext_t *tlsCtx)
{
  /* Initialize mbedTLS stuff */
  mbedtls_net_init(&tlsCtx->serverFd);
  mbedtls_ssl_init(&tlsCtx->ssl);
  mbedtls_ssl_config_init(&tlsCtx->conf);
  mbedtls_ctr_drbg_init(&tlsCtx->ctrDrbg);
  mbedtls_entropy_init(&tlsCtx->entropy);
}

void tlsShutdown(tlsClientContext_t *tlsCtx)
{
  /* Free mbedTLS stuff */
  mbedtls_ssl_close_notify(&tlsCtx->ssl);
  mbedtls_net_free(&tlsCtx->serverFd);
  mbedtls_ssl_free(&tlsCtx->ssl);
  mbedtls_ssl_config_free(&tlsCtx->conf);
  mbedtls_ctr_drbg_free(&tlsCtx->ctrDrbg);
  mbedtls_entropy_free(&tlsCtx->entropy);
}

int tlsConnect(tlsClientContext_t *tlsCtx, const char *host, uint32_t port,
               uint32_t timeout,
               const char *rootCA, size_t rootCASize, 
               const char *clientCA,  size_t clientCASize, 
               const char *privateKey,  size_t privateKeySize)
{
  int   ret;
  char *buf;

  TLSCDBG("Start tls_connect\n");

  /* Setup mbedTLS stuff */
  ret = mbedtls_ctr_drbg_seed(&tlsCtx->ctrDrbg, mbedtls_entropy_func,
                              &tlsCtx->entropy,
                              reinterpret_cast<const unsigned char*>(g_pers),
                              strlen(g_pers));
  if (ret != 0) {
    TLSCERR("mbedtls_ctr_drbg_seed() error : -0x%x\n", -ret);
    return ret;
  }

  if (rootCA) {
    TLSCDBG("Loading CA certificates\n");

    /* Setup certificates. */
    mbedtls_x509_crt_init(&tlsCtx->caCert);
    ret = mbedtls_x509_crt_parse(&tlsCtx->caCert,
                                 reinterpret_cast<const unsigned char*>(rootCA),
                                 rootCASize);
    if (ret != 0) {
      TLSCERR("mbedtls_x509_crt_parse() error : -0x%x\n", -ret);
      return ret;
    }
    mbedtls_ssl_conf_ca_chain(&tlsCtx->conf, &tlsCtx->caCert, NULL);
    mbedtls_ssl_conf_authmode(&tlsCtx->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
  }

  if (clientCA && privateKey) {
      /* Setup certificates. */
    mbedtls_x509_crt_init(&tlsCtx->cliCert);
    mbedtls_pk_init(&tlsCtx->cliKey);

    TLSCDBG("Loading client certificates\n");

    ret = mbedtls_x509_crt_parse(&tlsCtx->cliCert,
                                 reinterpret_cast<const unsigned char*>(clientCA),
                                 clientCASize);
    if (ret != 0) {
      TLSCERR("mbedtls_x509_crt_parse() error : -0x%x\n", -ret);
      return ret;
    }

    TLSCDBG("Loading private key\n");

    ret = mbedtls_pk_parse_key(&tlsCtx->cliKey,
                               reinterpret_cast<const unsigned char*>(privateKey),
                               privateKeySize, NULL, 0);
    if (ret != 0) {
      TLSCERR("mbedtls_pk_parse_key() error : -0x%x\n", -ret);
      return ret;
    }
    ret = mbedtls_ssl_conf_own_cert(&tlsCtx->conf, &tlsCtx->cliCert,
                                    &tlsCtx->cliKey);
    if (ret != 0) {
      TLSCERR("mbedtls_ssl_conf_own_cert() error : -0x%x\n", -ret);
      return ret;
    }
  }

  ret = mbedtls_ssl_config_defaults(&tlsCtx->conf, MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0) {
    TLSCERR("mbedtls_ssl_config_defaults() error : -0x%x\n", -ret);
    return ret;
  }

  mbedtls_ssl_conf_rng(&tlsCtx->conf, mbedtls_ctr_drbg_random,
                       &tlsCtx->ctrDrbg);
  mbedtls_ssl_conf_read_timeout(&tlsCtx->conf, timeout);
  mbedtls_ssl_setup(&tlsCtx->ssl, &tlsCtx->conf);
  ret = mbedtls_ssl_set_hostname(&tlsCtx->ssl, host);
  if (ret != 0) {
    TLSCERR("mbedtls_ssl_set_hostname() error : -0x%x\n", -ret);
    return ret;
  }

  TLSCDBG("Connect to server\n");

  String portStr(port);

  /* Start the connection.
   * mbedtls_net_connect execute address resolution, socket create,
   * and connect.
   */
  ret = mbedtls_net_connect(&tlsCtx->serverFd, host, portStr.c_str(),
                            MBEDTLS_NET_PROTO_TCP);
  if (ret != 0) {
    TLSCERR("mbedtls_net_connect() error : -0x%x\n", -ret);
    return ret;
  }

  mbedtls_ssl_set_bio(&tlsCtx->ssl, &tlsCtx->serverFd,
                      mbedtls_net_send, mbedtls_net_recv, NULL);

  TLSCDBG("Performing the SSL/TLS handshake\n");

  /* Do SSL handshake */
  while ((ret = mbedtls_ssl_handshake(&tlsCtx->ssl)) != 0) {
    if ((ret != MBEDTLS_ERR_SSL_WANT_READ) &&
        (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
      TLSCERR("mbedtls_ssl_handshake() error : -0x%x\n", -ret);
      return ret;
    }
  }

  TLSCDBG("Verify peer X.509 certificates\n");

  ret = mbedtls_ssl_get_verify_result(&tlsCtx->ssl);
  if (ret != 0) {
    buf = new char[BUF_LEN];
    if (!buf) {
      TLSCERR("failed to allocate memory\n");
      return -1;
    }
    memset(buf, 0, BUF_LEN);
    mbedtls_x509_crt_verify_info(buf, BUF_LEN, " ", ret);
    TLSCERR("Failed to verify perr certificates: %s\n", buf);
    delete[] buf;
    return -1;
  }
  TLSCDBG("Verified peer X.509 certificates\n");

  if (rootCA) {
    mbedtls_x509_crt_free(&tlsCtx->caCert);
  }

  if (clientCA && privateKey) {
    mbedtls_x509_crt_free(&tlsCtx->cliCert);
    mbedtls_pk_free(&tlsCtx->cliKey);
  }

  TLSCDBG("tls_connect done\n");

  return 0;
}

int tlsGetAvailable(tlsClientContext_t *tlsCtx)
{
  /* mbedtls_ssl_read() must be called before mbedtls_ssl_get_bytes_avail().
   * Because if don't do this, mbedtls_ssl_get_bytes_avail() will
   * always return with 0. */
  mbedtls_ssl_read(&tlsCtx->ssl, NULL, 0);

  return mbedtls_ssl_get_bytes_avail(&tlsCtx->ssl);
}

int tlsRead(tlsClientContext_t *tlsCtx, uint8_t *buffer, int len)
{
  int ret;

  while ((ret = mbedtls_ssl_read(&tlsCtx->ssl, buffer, len)) <= 0) {
    if ((ret != MBEDTLS_ERR_SSL_WANT_READ) &&
        (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
      TLSCERR("mbedtls_ssl_read() error : -0x%x\n", -ret);
      return ret;
    }
  }

  return ret;
}

int tlsWrite(tlsClientContext_t *tlsCtx, const uint8_t *buffer, int len,
             uint32_t timeout)
{
  int ret;
  struct timespec timer;

  startTimer(&timer);

  while ((ret = mbedtls_ssl_write(&tlsCtx->ssl, buffer, len)) <= 0) {
    if ((ret != MBEDTLS_ERR_SSL_WANT_READ) &&
        (ret != MBEDTLS_ERR_SSL_WANT_WRITE)) {
      TLSCERR("mbedtls_ssl_write() error : -0x%x\n", -ret);
      return ret;
    } else if (hasTimerExpired(&timer, timeout)) {
      TLSCERR("write timer expired : -0x%x\n", -ret);
      return ret;
    }
  }

  return ret;
}
