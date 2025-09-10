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

#include "tftp.h"
#include "netutil.h"

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

/* TFTP */
#define TFTP_SOCKET_ID 1
#define TFTP_CLIENT_SOCKET_BUFFER_SIZE 2048
#define TFTP_SERVER_IP "192.168.11.2"
#define TFTP_SERVER_FILE_NAME "tftp_test_file.txt"


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
/* TFTP */
uint8_t tftp_client_socket_buffer[TFTP_CLIENT_SOCKET_BUFFER_SIZE] = {0};

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
    int tftp_state;
    uint8_t tftp_read_flag = 0;
    uint32_t tftp_server_ip = inet_addr((uint8_t *)TFTP_SERVER_IP);
    uint8_t tftp_read_file_name[] = TFTP_SERVER_FILE_NAME;

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

    // start_ms = millis();

    TFTP_init(TFTP_SOCKET_ID, tftp_client_socket_buffer);

    /* Infinite loop */
    while (1) {
        if (tftp_read_flag == 0) {
            printf("tftp server ip: %s, file name: %s\r\n", TFTP_SERVER_IP, TFTP_SERVER_FILE_NAME);
            printf("send request\r\n");
            TFTP_read_request(tftp_server_ip, (uint8_t *)TFTP_SERVER_FILE_NAME);
            tftp_read_flag = 1;
        } else {
            tftp_state = TFTP_run();
            if (tftp_state == TFTP_SUCCESS) {
                printf("tftp read success, file name: %s\r\n", tftp_read_file_name);
                while (1) {
                }
            } else if (tftp_state == TFTP_FAIL) {
                printf("tftp read fail, file name: %s\r\n", tftp_read_file_name);
                while (1) {
                }
            }
        }
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
    tftp_timeout_handler();
}

// static time_t millis(void)
// {
//     return g_msec_cnt;
// }
