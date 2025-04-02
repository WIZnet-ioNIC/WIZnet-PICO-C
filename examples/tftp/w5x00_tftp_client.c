/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
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
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

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
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 10},                   // IP address
        .sn = {255, 255, 255, 0},                  // Subnet Mask
        .gw = {192, 168, 11, 1},                   // Gateway
        .dns = {168, 126, 63, 1},                    // DNS server
        .dhcp = NETINFO_STATIC                      // DHCP enable/disable
};
/* TFTP */
uint8_t tftp_client_socket_buffer[TFTP_CLIENT_SOCKET_BUFFER_SIZE] = {0};

/* Timer */
static volatile uint32_t g_msec_cnt = 0;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/* Timer  */
static void repeating_timer_callback(void);
static time_t millis(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    uint8_t retval = 0;
    uint32_t start_ms = 0;

    int tftp_state;
    uint8_t tftp_read_flag = 0;
    uint32_t tftp_server_ip = inet_addr(TFTP_SERVER_IP);
    uint8_t tftp_read_file_name[] = TFTP_SERVER_FILE_NAME;

    set_clock_khz();

    stdio_init_all();

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
    while (1)
    {
        if (tftp_read_flag == 0)
        {
            printf("tftp server ip: %s, file name: %s\r\n", TFTP_SERVER_IP, TFTP_SERVER_FILE_NAME);
            printf("send request\r\n");
            TFTP_read_request(tftp_server_ip, TFTP_SERVER_FILE_NAME);
            tftp_read_flag = 1;
        }
        else
        {
            tftp_state = TFTP_run();
            if (tftp_state == TFTP_SUCCESS)
            {
                printf("tftp read success, file name: %s\r\n", tftp_read_file_name);
                while (1)
                {
                }
            }
            else if (tftp_state == TFTP_FAIL)
            {
                printf("tftp read fail, file name: %s\r\n", tftp_read_file_name);
                while (1)
                {
                }
            }
        }
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

/* Timer */
static void repeating_timer_callback(void)
{
    g_msec_cnt++;
    tftp_timeout_handler();
}

// static time_t millis(void)
// {
//     return g_msec_cnt;
// }
