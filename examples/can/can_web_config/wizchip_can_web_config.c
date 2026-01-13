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
#include "wizchip_gpio_irq.h"

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/irq.h"
#include "hardware/structs/pio.h"

#include "web_page.h"

#include "httpServer.h"
#include "can_to_eth.h"
#include "canHandler.h"

/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/
/* Buffer */
#define HTTP_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define HTTP_SOCKET_MAX_NUM 4


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

/* HTTP */
static uint8_t g_http_send_buf[HTTP_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_http_recv_buf[HTTP_BUF_MAX_SIZE] = {
    0,
};

uint8_t g_tx_buf[DATA_BUF_MAX_SIZE] = {
    0,
};
uint8_t g_rx_buf[DATA_BUF_MAX_SIZE] = {
    0,
};

static uint8_t g_http_socket_num_list[HTTP_SOCKET_MAX_NUM] = {4, 5, 6, 7};

static uint8_t destip[4] = {192, 168, 11, 3};
static uint16_t destport = 5000;

uint8_t can_config_changed = 0;

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/

/**
    ----------------------------------------------------------------------------------------------------
    Main
    ----------------------------------------------------------------------------------------------------
*/
int main() {
    /* Initialize */
    uint8_t i = 0;
    int retval = 0;
    CanConfig *p_can_config;

    stdio_init_all();
    sleep_ms(3000);
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);


    p_can_config = get_CanConfig_pointer();
    get_default_can_config(p_can_config);
    printCanConfig(p_can_config);

    if (can_initialize(p_can_config) < 0) {
        printf("CAN setup failed...\n");
        while (1) {}
    }

    httpServer_init(g_http_send_buf, g_http_recv_buf, HTTP_SOCKET_MAX_NUM, g_http_socket_num_list);

    /* Register web page */
    reg_httpServer_webContent("index.html", _acindex);

    /* Infinite loop */
    while (1) {
        /* Run HTTP server */
        for (i = 0; i < HTTP_SOCKET_MAX_NUM; i++) {
            httpServer_run(i);
        }

        if (can_config_changed) {
            reset_eth();

            CanConfig *can_config = get_CanConfig_pointer();
            printCanConfig(can_config);
            can_restart(can_config);

            can_config_changed = 0;
        }

        if ((retval = can_to_eth(p_can_config->eth_mode)) < 0) {
            printf(" CAN to Eth loopback error : %d\n", retval);

            while (1)
                ;
        }

    }

}
