/**   * Copyright (c) 2021 WIZnet Co.,Ltd

    SPDX-License-Identifier: BSD-3-Clause
*/

/**
    ----------------------------------------------------------------------------------------------------
    Includes
    ----------------------------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "wizchip_spi.h"

#include "timer.h"
#include "time.h"

#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "mbedtls/pk.h"
#include "mbedtls/ctr_drbg.h"

/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/
/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_SSL 0

/* Port */
#define PORT_SSL 443

/* Allow only RSA key-exchange suites (no DHE params available) */
static const int g_ciphersuites[] = {
    MBEDTLS_TLS_RSA_WITH_AES_256_CBC_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_128_GCM_SHA256,
    MBEDTLS_TLS_RSA_WITH_AES_128_CBC_SHA256,
    0
};

/**
    ----------------------------------------------------------------------------------------------------
    Variables
    ----------------------------------------------------------------------------------------------------
*/
/* Network */
static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 11, 2},                     // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
#if _WIZCHIP_ > W5500
    .lla = {
        0xfe, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x08, 0xdc, 0xff,
        0xfe, 0x57, 0x57, 0x25
    },             // Link Local Address
    .gua = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },             // Global Unicast Address
    .sn6 = {
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },             // IPv6 Prefix
    .gw6 = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    },             // Gateway IPv6 Address
    .dns6 = {
        0x20, 0x01, 0x48, 0x60,
        0x48, 0x60, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x88, 0x88
    },             // DNS6 server
    .ipmode = NETINFO_STATIC_ALL
#else
    .dhcp = NETINFO_STATIC
#endif
};

/* SSL */
static uint8_t g_ssl_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

static mbedtls_ctr_drbg_context g_ctr_drbg;
static mbedtls_ssl_config g_conf;
static mbedtls_ssl_context g_ssl;
static mbedtls_x509_crt g_srv_cert;
static mbedtls_pk_context g_srv_key;
static int g_handshake_done = 0;

/* Embedded test RSA server cert/key (from mbedtls tests) */
static const unsigned char g_srv_crt_pem[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDNzCCAh+gAwIBAgIBAjANBgkqhkiG9w0BAQsFADA7MQswCQYDVQQGEwJOTDER\r\n"
    "MA8GA1UECgwIUG9sYXJTU0wxGTAXBgNVBAMMEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"
    "MTkwMjEwMTQ0NDA2WhcNMjkwMjEwMTQ0NDA2WjA0MQswCQYDVQQGEwJOTDERMA8G\r\n"
    "A1UECgwIUG9sYXJTU0wxEjAQBgNVBAMMCWxvY2FsaG9zdDCCASIwDQYJKoZIhvcN\r\n"
    "AQEBBQADggEPADCCAQoCggEBAMFNo93nzR3RBNdJcriZrA545Do8Ss86ExbQWuTN\r\n"
    "owCIp+4ea5anUrSQ7y1yej4kmvy2NKwk9XfgJmSMnLAofaHa6ozmyRyWvP7BBFKz\r\n"
    "NtSj+uGxdtiQwWG0ZlI2oiZTqqt0Xgd9GYLbKtgfoNkNHC1JZvdbJXNG6AuKT2kM\r\n"
    "tQCQ4dqCEGZ9rlQri2V5kaHiYcPNQEkI7mgM8YuG0ka/0LiqEQMef1aoGh5EGA8P\r\n"
    "hYvai0Re4hjGYi/HZo36Xdh98yeJKQHFkA4/J/EwyEoO79bex8cna8cFPXrEAjya\r\n"
    "HT4P6DSYW8tzS1KW2BGiLICIaTla0w+w3lkvEcf36hIBMJcCAwEAAaNNMEswCQYD\r\n"
    "VR0TBAIwADAdBgNVHQ4EFgQUpQXoZLjc32APUBJNYKhkr02LQ5MwHwYDVR0jBBgw\r\n"
    "FoAUtFrkpbPe0lL2udWmlQ/rPrzH/f8wDQYJKoZIhvcNAQELBQADggEBAC465FJh\r\n"
    "Pqel7zJngHIHJrqj/wVAxGAFOTF396XKATGAp+HRCqJ81Ry60CNK1jDzk8dv6M6U\r\n"
    "HoS7RIFiM/9rXQCbJfiPD5xMTejZp5n5UYHAmxsxDaazfA5FuBhkfokKK6jD4Eq9\r\n"
    "1C94xGKb6X4/VkaPF7cqoBBw/bHxawXc0UEPjqayiBpCYU/rJoVZgLqFVP7Px3sv\r\n"
    "a1nOrNx8rPPI1hJ+ZOg8maiPTxHZnBVLakSSLQy/sWeWyazO1RnrbxjrbgQtYKz0\r\n"
    "e3nwGpu1w13vfckFmUSBhHXH7AAS/HpKC4IH7G2GAk3+n8iSSN71sZzpxonQwVbo\r\n"
    "pMZqLmbBm/7WPLc=\r\n"
    "-----END CERTIFICATE-----\r\n";

static const unsigned char g_srv_key_pem[] =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIIEpAIBAAKCAQEAwU2j3efNHdEE10lyuJmsDnjkOjxKzzoTFtBa5M2jAIin7h5r\r\n"
    "lqdStJDvLXJ6PiSa/LY0rCT1d+AmZIycsCh9odrqjObJHJa8/sEEUrM21KP64bF2\r\n"
    "2JDBYbRmUjaiJlOqq3ReB30Zgtsq2B+g2Q0cLUlm91slc0boC4pPaQy1AJDh2oIQ\r\n"
    "Zn2uVCuLZXmRoeJhw81ASQjuaAzxi4bSRr/QuKoRAx5/VqgaHkQYDw+Fi9qLRF7i\r\n"
    "GMZiL8dmjfpd2H3zJ4kpAcWQDj8n8TDISg7v1t7HxydrxwU9esQCPJodPg/oNJhb\r\n"
    "y3NLUpbYEaIsgIhpOVrTD7DeWS8Rx/fqEgEwlwIDAQABAoIBAQCXR0S8EIHFGORZ\r\n"
    "++AtOg6eENxD+xVs0f1IeGz57Tjo3QnXX7VBZNdj+p1ECvhCE/G7XnkgU5hLZX+G\r\n"
    "Z0jkz/tqJOI0vRSdLBbipHnWouyBQ4e/A1yIJdlBtqXxJ1KE/ituHRbNc4j4kL8Z\r\n"
    "/r6pvwnTI0PSx2Eqs048YdS92LT6qAv4flbNDxMn2uY7s4ycS4Q8w1JXnCeaAnYm\r\n"
    "WYI5wxO+bvRELR2Mcz5DmVnL8jRyml6l6582bSv5oufReFIbyPZbQWlXgYnpu6He\r\n"
    "GTc7E1zKYQGG/9+DQUl/1vQuCPqQwny0tQoX2w5tdYpdMdVm+zkLtbajzdTviJJa\r\n"
    "TWzL6lt5AoGBAN86+SVeJDcmQJcv4Eq6UhtRr4QGMiQMz0Sod6ettYxYzMgxtw28\r\n"
    "CIrgpozCc+UaZJLo7UxvC6an85r1b2nKPCLQFaggJ0H4Q0J/sZOhBIXaoBzWxveK\r\n"
    "nupceKdVxGsFi8CDy86DBfiyFivfBj+47BbaQzPBj7C4rK7UlLjab2rDAoGBAN2u\r\n"
    "AM2gchoFiu4v1HFL8D7lweEpi6ZnMJjnEu/dEgGQJFjwdpLnPbsj4c75odQ4Gz8g\r\n"
    "sw9lao9VVzbusoRE/JGI4aTdO0pATXyG7eG1Qu+5Yc1YGXcCrliA2xM9xx+d7f+s\r\n"
    "mPzN+WIEg5GJDYZDjAzHG5BNvi/FfM1C9dOtjv2dAoGAF0t5KmwbjWHBhcVqO4Ic\r\n"
    "BVvN3BIlc1ue2YRXEDlxY5b0r8N4XceMgKmW18OHApZxfl8uPDauWZLXOgl4uepv\r\n"
    "whZC3EuWrSyyICNhLY21Ah7hbIEBPF3L3ZsOwC+UErL+dXWLdB56Jgy3gZaBeW7b\r\n"
    "vDrEnocJbqCm7IukhXHOBK8CgYEAwqdHB0hqyNSzIOGY7v9abzB6pUdA3BZiQvEs\r\n"
    "3LjHVd4HPJ2x0N8CgrBIWOE0q8+0hSMmeE96WW/7jD3fPWwCR5zlXknxBQsfv0gP\r\n"
    "3BC5PR0Qdypz+d+9zfMf625kyit4T/hzwhDveZUzHnk1Cf+IG7Q+TOEnLnWAWBED\r\n"
    "ISOWmrUCgYAFEmRxgwAc/u+D6t0syCwAYh6POtscq9Y0i9GyWk89NzgC4NdwwbBH\r\n"
    "4AgahOxIxXx2gxJnq3yfkJfIjwf0s2DyP0kY2y6Ua1OeomPeY9mrIS4tCuDQ6LrE\r\n"
    "TB6l9VGoxJL4fyHnZb8L5gGvnB1bbD8cL6YPaDiOhcRseC9vBiEuVg==\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";

static const size_t g_srv_crt_pem_len = sizeof(g_srv_crt_pem);
static const size_t g_srv_key_pem_len = sizeof(g_srv_key_pem);

/* Timer  */
static volatile uint32_t g_msec_cnt = 0;

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* SSL */
static int wizchip_ssl_init(uint8_t socket_fd);
static void ssl_reset_session(uint8_t socket_fd);
static int ssl_handshake_blocking(void);
static int ssl_load_credentials(void);
static int ssl_random_callback(void *p_rng, unsigned char *output, size_t output_len);
static int tcp_send(void *ctx, const unsigned char *buf, size_t len);
static int tcp_recv(void *ctx, unsigned char *buf, size_t len);
static int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout);

/* Timer  */
static void repeating_timer_callback(void);
static time_t millis(void);

/**
    ----------------------------------------------------------------------------------------------------
    Main
    ----------------------------------------------------------------------------------------------------
*/
int main() {
    const int *list = 0;
    uint16_t len = 0;
    uint32_t retval = 0;
    const uint8_t socket_id = SOCKET_SSL;

    stdio_init_all();
    sleep_ms(3000);
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    /* Seed RNG after timer starts so millis() is non-zero */
    srand((unsigned)millis());

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    retval = wizchip_ssl_init(socket_id);

    if (retval < 0) {
        printf(" SSL initialize failed %d\n", retval);

        while (1)
            ;
    }

    /* Get ciphersuite information */
    printf(" Configured server ciphersuites\n");

    list = g_ciphersuites;

    while (*list) {
        printf(" %s\n", mbedtls_ssl_get_ciphersuite_name(*list));
        list++;
    }

    retval = socket(socket_id, Sn_MR_TCP, PORT_SSL, SF_TCP_NODELAY);

    if (retval != SOCKET_SSL) {
        printf(" Socket failed %d\n", retval);

        while (1)
            ;
    }

    retval = listen(socket_id);

    if (retval != SOCK_OK) {
        printf(" Listen failed %d\n", retval);

        while (1)
            ;
    }

    printf(" SSL server listening on port %d\n", PORT_SSL);

    /* Server loop */
    while (1) {
        switch (getSn_SR(socket_id)) {
        case SOCK_ESTABLISHED:
            if (getSn_IR(socket_id) & Sn_IR_CON) {
                setSn_IR(socket_id, Sn_IR_CON);
                ssl_reset_session(socket_id);
                g_handshake_done = 0;
                printf(" TCP connection accepted\n");
            }

            if (!g_handshake_done) {
                retval = ssl_handshake_blocking();

                if (retval != 0) {
                    char err_buf[128];
                    mbedtls_strerror(retval, err_buf, sizeof(err_buf));
                    printf(" SSL handshake failed -0x%x (%s)\n", -retval, err_buf);

                    mbedtls_ssl_close_notify(&g_ssl);
                    disconnect(socket_id);
                    close(socket_id);
                    g_handshake_done = 0;

                    break;
                }

                g_handshake_done = 1;
                printf(" SSL handshake complete, ciphersuite %s\n", mbedtls_ssl_get_ciphersuite(&g_ssl));

                const char *msg = " W5x00 SSL server ready\r\n";
                mbedtls_ssl_write(&g_ssl, (const unsigned char *)msg, strlen(msg));
            }

            getsockopt(socket_id, SO_RECVBUF, &len);

            if (len > 0) {
                if (len >= ETHERNET_BUF_MAX_SIZE) {
                    len = ETHERNET_BUF_MAX_SIZE - 1;
                }

                memset(g_ssl_buf, 0x00, ETHERNET_BUF_MAX_SIZE);

                retval = mbedtls_ssl_read(&g_ssl, g_ssl_buf, len);

                if (retval > 0) {
                    g_ssl_buf[retval] = 0x00;
                    printf(" Received: %s", g_ssl_buf);

                    mbedtls_ssl_write(&g_ssl, g_ssl_buf, retval);
                } else if ((retval == 0) ||
                           ((retval != MBEDTLS_ERR_SSL_WANT_READ) && (retval != MBEDTLS_ERR_SSL_WANT_WRITE))) {
                    printf(" SSL read error %d\n", retval);

                    mbedtls_ssl_close_notify(&g_ssl);
                    disconnect(socket_id);
                    close(socket_id);
                    g_handshake_done = 0;
                }
            }

            break;
        case SOCK_CLOSE_WAIT:
            mbedtls_ssl_close_notify(&g_ssl);
            disconnect(socket_id);
            close(socket_id);
            g_handshake_done = 0;
            printf(" Connection closed, waiting for new client\n");
            break;
        case SOCK_CLOSED:
            g_handshake_done = 0;
            ssl_reset_session(socket_id);

            if (socket(socket_id, Sn_MR_TCP, PORT_SSL, SF_TCP_NODELAY) == SOCKET_SSL) {
                if (listen(socket_id) == SOCK_OK) {
                    printf(" Re-listening on port %d\n", PORT_SSL);
                }
            }
            break;
        default:
            sleep_ms(1);
            break;
        }
    }
}

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* SSL */
static int wizchip_ssl_init(uint8_t socket_fd) {
    int retval;

    mbedtls_ctr_drbg_init(&g_ctr_drbg);
    mbedtls_ssl_init(&g_ssl);
    mbedtls_ssl_config_init(&g_conf);
    mbedtls_x509_crt_init(&g_srv_cert);
    mbedtls_pk_init(&g_srv_key);

    retval = mbedtls_ssl_config_defaults(&g_conf,
              MBEDTLS_SSL_IS_SERVER,
              MBEDTLS_SSL_TRANSPORT_STREAM,
              MBEDTLS_SSL_PRESET_DEFAULT);

    if (retval != 0) {
        printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n", retval);

        return -1;
    }

    mbedtls_ssl_conf_authmode(&g_conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&g_conf, ssl_random_callback, &g_ctr_drbg);
        mbedtls_ssl_conf_ciphersuites(&g_conf, g_ciphersuites);
    mbedtls_ssl_conf_endpoint(&g_conf, MBEDTLS_SSL_IS_SERVER);
    mbedtls_ssl_conf_read_timeout(&g_conf, 1000 * 10);

    if ((retval = ssl_load_credentials()) != 0) {
        return -1;
    }

    if ((retval = mbedtls_ssl_setup(&g_ssl, &g_conf)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n", retval);

        return -1;
    }

    mbedtls_ssl_set_bio(&g_ssl, (void *)(uintptr_t)socket_fd, tcp_send, tcp_recv, recv_timeout);

    return 1;
}

static void ssl_reset_session(uint8_t socket_fd) {
    int retval = mbedtls_ssl_session_reset(&g_ssl);

    if (retval != 0) {
        printf(" SSL session reset failed %d\n", retval);

        return;
    }

    mbedtls_ssl_set_bio(&g_ssl, (void *)(uintptr_t)socket_fd, tcp_send, tcp_recv, recv_timeout);
}

static int ssl_handshake_blocking(void) {
    int retval;

    do {
        retval = mbedtls_ssl_handshake(&g_ssl);
    } while ((retval == MBEDTLS_ERR_SSL_WANT_READ) || (retval == MBEDTLS_ERR_SSL_WANT_WRITE));

    return retval;
}

static int ssl_load_credentials(void) {
    int retval;

    retval = mbedtls_x509_crt_parse(&g_srv_cert, g_srv_crt_pem, g_srv_crt_pem_len);

    if (retval < 0) {
        printf(" failed\n  ! mbedtls_x509_crt_parse returned -0x%x\n", -retval);

        return retval;
    }

    retval = mbedtls_pk_parse_key(&g_srv_key,
             g_srv_key_pem,
             g_srv_key_pem_len,
             0,
             0,
             ssl_random_callback,
             &g_ctr_drbg);

    if (retval != 0) {
        printf(" failed\n  ! mbedtls_pk_parse_key returned -0x%x\n", -retval);

        return retval;
    }

    retval = mbedtls_ssl_conf_own_cert(&g_conf, &g_srv_cert, &g_srv_key);

    if (retval != 0) {
        printf(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n", retval);

        return retval;
    }

    return 0;
}

static int ssl_random_callback(void *p_rng, unsigned char *output, size_t output_len) {
    int i;

    (void)p_rng;

    if (output_len <= 0) {
        return 1;
    }

    for (i = 0; i < output_len; i++) {
        *output++ = rand() % 0xff;
    }

    srand(rand());

    return 0;
}

static int tcp_send(void *ctx, const unsigned char *buf, size_t len) {
    return send((uint8_t)(uintptr_t)ctx, (uint8_t *)buf, (uint16_t)len);
}

static int tcp_recv(void *ctx, unsigned char *buf, size_t len) {
    return recv((uint8_t)(uintptr_t)ctx, (uint8_t *)buf, (uint16_t)len);
}

static int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout) {
    uint16_t recv_len = 0;
    uint32_t start_ms = millis();

    do {
        getsockopt((uint8_t)(uintptr_t)(ctx), SO_RECVBUF, &recv_len);

        if (recv_len > 0) {
            return recv((uint8_t)(uintptr_t)ctx, (uint8_t *)buf, (uint16_t)len);
        }
    } while ((millis() - start_ms) < timeout);

    return MBEDTLS_ERR_SSL_TIMEOUT;
}

/* Timer */
static void repeating_timer_callback(void) {
    g_msec_cnt++;
}

static time_t millis(void) {
    return g_msec_cnt;
}