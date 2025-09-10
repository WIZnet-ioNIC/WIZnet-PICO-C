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

#include "hyperterminal.h"
#include "MakeXML.h"
#include "UPnP.h"

/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/
/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 4)

/* Socket */
#define SOCKET_UPNP 0

/* Port */
#define PORT_TCP 8000
#define PORT_UDP 5000

/* USER LEDS */
#if DEVICE_BOARD_NAME == W55RP20_EVB_PICO
#define USER_LED_PIN     19
#else
#define USER_LED_PIN     25
#endif

#if (_WIZCHIP_ == W5100S)
static uint8_t tx_size[_WIZCHIP_SOCK_NUM_] = {4, 2, 2, 0};
static uint8_t rx_size[_WIZCHIP_SOCK_NUM_] = {4, 2, 2, 0};
#elif (_WIZCHIP_ >= W5500)
static uint8_t tx_size[_WIZCHIP_SOCK_NUM_] = {4, 4, 2, 1, 1, 1, 1, 2};
static uint8_t rx_size[_WIZCHIP_SOCK_NUM_] = {4, 4, 2, 1, 1, 1, 1, 2};
#endif

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

/* UPNP */
static uint8_t g_upnp_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* USER LEDS */
static void UserLED_Init(void);
static void setUserLEDStatus(uint8_t val);

/**
    ----------------------------------------------------------------------------------------------------
    Main
    ----------------------------------------------------------------------------------------------------
*/
int main() {
    /* Initialize */
    stdio_init_all();

    while (!stdio_usb_connected());

    printf("wiznet chip upnp example.\r\n");

    UserLED_Init();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_init(tx_size, rx_size);

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    do {
        printf("Send SSDP.. \r\n");
    } while (SSDPProcess(SOCKET_UPNP) != 0); // SSDP Search discovery

    if (GetDescriptionProcess(SOCKET_UPNP) == 0) { // GET IGD description
        printf("GetDescription Success!!\r\n");
    } else {
        printf("GetDescription Fail!!\r\n");
    }

    if (SetEventing(SOCKET_UPNP) == 0) { // Subscribes IGD event messageS
        printf("SetEventing Success!!\r\n");
    } else {
        printf("SetEventing Fail!!\r\n");
    }

    Main_Menu(SOCKET_UPNP, SOCKET_UPNP + 1, SOCKET_UPNP + 2, g_upnp_buf, PORT_TCP, PORT_UDP); // Main menu

    /* Infinite loop */
    while (1) {
    }
}

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
void UserLED_Init() {
    gpio_init(USER_LED_PIN);              // Initialize LED
    gpio_set_dir(USER_LED_PIN, GPIO_OUT); // Output mode
    UserLED_Control_Init(setUserLEDStatus);
}

void setUserLEDStatus(uint8_t val) {
    if (val == 0) {
        gpio_put(USER_LED_PIN, 0);
    } else {
        gpio_put(USER_LED_PIN, 1);
    }
}