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
static uint8_t g_ssl_target_ip[4] = {192, 168, 11, 4};

static mbedtls_ctr_drbg_context g_ctr_drbg;
static mbedtls_ssl_config g_conf;
static mbedtls_ssl_context g_ssl;

/* Timer  */
static volatile uint32_t g_msec_cnt = 0;

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* SSL */
static int wizchip_ssl_init(uint8_t *socket_fd);
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
    /* Initialize */
    const int *list = NULL;
    uint16_t len = 0;
    uint32_t retval = 0;
    uint32_t start_ms = 0;

    stdio_init_all();
    sleep_ms(3000);
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    retval = wizchip_ssl_init((void *)SOCKET_SSL);

    if (retval < 0) {
        printf(" SSL initialize failed %d\n", retval);

        while (1)
            ;
    }

    /* Get ciphersuite information */
    printf(" Supported ciphersuite lists\n");

    list = mbedtls_ssl_list_ciphersuites();

    while (*list) {
        printf(" %-42s\n", mbedtls_ssl_get_ciphersuite_name(*list));

        list++;

        if (!*list) {
            break;
        }

        printf(" %s\n", mbedtls_ssl_get_ciphersuite_name(*list));

        list++;
    }

    retval = socket((uint8_t)(uintptr_t)(g_ssl.private_p_bio), Sn_MR_TCP, PORT_SSL, SF_TCP_NODELAY);

    if (retval != SOCKET_SSL) {
        printf(" Socket failed %d\n", retval);

        while (1)
            ;
    }

    start_ms = millis();

    do {
        retval = connect((uint8_t)(uintptr_t)(g_ssl.private_p_bio), g_ssl_target_ip, PORT_SSL);

        if ((retval == SOCK_OK) || (retval == SOCKERR_TIMEOUT)) {
            break;
        }
    } while ((millis() - start_ms) < RECV_TIMEOUT);

    if ((retval != SOCK_OK) || (retval == SOCK_BUSY)) {
        printf(" Connect failed %d\n", retval);

        while (1)
            ;
    }

    printf(" Connected %d\n", retval);

    while ((retval = mbedtls_ssl_handshake(&g_ssl)) != 0) {
        if ((retval != MBEDTLS_ERR_SSL_WANT_READ) && (retval != MBEDTLS_ERR_SSL_WANT_WRITE)) {
            printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n", -retval);

            while (1)
                ;
        }
    }

    printf(" ok\n    [ Ciphersuite is %s ]\n", mbedtls_ssl_get_ciphersuite(&g_ssl));

    memset(g_ssl_buf, 0x00, ETHERNET_BUF_MAX_SIZE);
    const char *msg = " W5x00 TCP over SSL test\n";
    memcpy(g_ssl_buf, msg, strlen(msg));

    mbedtls_ssl_write(&g_ssl, g_ssl_buf, strlen(msg));

    /* Infinite loop */
    while (1) {
        getsockopt((uint8_t)(uintptr_t)(g_ssl.private_p_bio), SO_RECVBUF, &len);

        if (len > 0) {
            if (len > ETHERNET_BUF_MAX_SIZE) {
                len = ETHERNET_BUF_MAX_SIZE;
            }

            memset(g_ssl_buf, 0x00, ETHERNET_BUF_MAX_SIZE);

            mbedtls_ssl_read(&g_ssl, g_ssl_buf, len);

            printf("%s", g_ssl_buf);
        }
    }
}

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* SSL */
static int wizchip_ssl_init(uint8_t *socket_fd) {
    int retval;

    mbedtls_ctr_drbg_init(&g_ctr_drbg);
    mbedtls_ssl_init(&g_ssl);
    mbedtls_ssl_config_init(&g_conf);

    if ((retval = mbedtls_ssl_config_defaults(&g_conf,
                  MBEDTLS_SSL_IS_CLIENT,
                  MBEDTLS_SSL_TRANSPORT_STREAM,
                  MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n", retval);

        return -1;
    }

    printf(" Socket descriptor %d\n", socket_fd);

    mbedtls_ssl_conf_authmode(&g_conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&g_conf, ssl_random_callback, &g_ctr_drbg);
    mbedtls_ssl_conf_endpoint(&g_conf, MBEDTLS_SSL_IS_CLIENT);
    mbedtls_ssl_conf_read_timeout(&g_conf, 1000 * 10);

    if ((retval = mbedtls_ssl_setup(&g_ssl, &g_conf)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n", retval);

        return -1;
    }

    mbedtls_ssl_set_bio(&g_ssl, socket_fd, tcp_send, tcp_recv, recv_timeout);

    return 1;
}

static int ssl_random_callback(void *p_rng, unsigned char *output, size_t output_len) {
    int i;

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