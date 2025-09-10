/**
    Copyright (c) 2021 WIZnet Co.,Ltd

    SPDX-License-Identifier: BSD-3-Clause
*/

/**
    ----------------------------------------------------------------------------------------------------
    Includes
    ----------------------------------------------------------------------------------------------------
*/
#include <stdio.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"

#include "sntp.h"

#include "timer.h"
#include "time.h"

/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_SNTP 0

/* Timeout */
#define RECV_TIMEOUT (1000 * 10) // 10 seconds

/* Timezone */
#define TIMEZONE 40 // Korea

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

/* SNTP */
static uint8_t g_sntp_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_sntp_server_ip[4] = {216, 239, 35, 0}; // time.google.com

/* Timer */
static volatile uint32_t g_msec_cnt = 0;

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/

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
    uint8_t retval = 0;
    uint32_t start_ms = 0;
    datetime time;

    stdio_init_all();
    sleep_ms(3000);
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    network_initialize(g_net_info);

    SNTP_init(SOCKET_SNTP, g_sntp_server_ip, TIMEZONE, g_sntp_buf);

    /* Get network information */
    print_network_information(g_net_info);

    start_ms = millis();

    /* Get time */
    do {
        retval = SNTP_run(&time);

        if (retval == 1) {
            break;
        }
    } while ((millis() - start_ms) < RECV_TIMEOUT);

    if (retval != 1) {
        printf(" SNTP failed : %d\n", retval);

        while (1)
            ;
    }

    printf(" %d-%d-%d, %d:%d:%d\n", time.yy, time.mo, time.dd, time.hh, time.mm, time.ss);

    /* Infinite loop */
    while (1) {
        ; // nothing to do
    }
}

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/

/* Timer */
static void repeating_timer_callback(void) {
    g_msec_cnt++;
}

static time_t millis(void) {
    return g_msec_cnt;
}
