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

#include "PPPoE.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* PPPoE */
#define DATA_BUF_SIZE 2048

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
#if _WIZCHIP_ > W5500
        .lla = {0xfe, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x02, 0x08, 0xdc, 0xff,
                0xfe, 0x57, 0x57, 0x25},             // Link Local Address
        .gua = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Global Unicast Address
        .sn6 = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // IPv6 Prefix
        .gw6 = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Gateway IPv6 Address
        .dns6 = {0x20, 0x01, 0x48, 0x60,
                0x48, 0x60, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x88, 0x88},             // DNS6 server
        .ipmode = NETINFO_STATIC_ALL
#else
        .dhcp = NETINFO_STATIC        
#endif
};

/* PPPoE */
uint8_t gDATABUF[DATA_BUF_SIZE];

uint8_t pppoe_id[6] = "W5100S";
uint8_t pppoe_id_len = 6;
uint8_t pppoe_pw[6] = "WIZnet";
uint8_t pppoe_pw_len = 6;
uint8_t pppoe_ip[4] = {
    0,
};

uint16_t pppoe_retry_count = 0;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int32_t ret = 0;
    uint8_t str[15];

    set_clock_khz();

    stdio_init_all();
    sleep_ms(3000);
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    printf("wiznet chip PPPOE example.\r\n");

    while (1)
    {
        ret = ppp_start(gDATABUF); // ppp start function

        if (ret == PPP_SUCCESS || pppoe_retry_count > PPP_MAX_RETRY_COUNT)
        {
            break; // PPPoE Connected or connect failed by over retry count
        }
            
    }
    if (ret == PPP_SUCCESS) // 1 : success
    {

        printf("\r\n<<<< PPPoE Success >>>>\r\n");
        printf("Assigned IP address : %d.%d.%d.%d\r\n", pppoe_ip[0], pppoe_ip[1], pppoe_ip[2], pppoe_ip[3]);

        printf("\r\n==================================================\r\n");
        printf("    AFTER PPPoE, Net Configuration Information        \r\n");
        printf("==================================================\r\n");

        getSHAR(str);
        printf("MAC address  : %x:%x:%x:%x:%x:%x\r\n", str[0], str[1], str[2], str[3], str[4], str[5]);
        getSUBR(str);
        printf("SUBNET MASK  : %d.%d.%d.%d\r\n", str[0], str[1], str[2], str[3]);
        getGAR(str);
        printf("G/W IP ADDRESS : %d.%d.%d.%d\r\n", str[0], str[1], str[2], str[3]);
        getSIPR(str);
        printf("SOURCE IP ADDRESS : %d.%d.%d.%d\r\n\r\n", str[0], str[1], str[2], str[3]);
    }
    else // failed
    {
        printf("\r\n<<<< PPPoE Failed >>>>\r\n");

        /* Get network information */
        print_network_information(g_net_info);
    }

    /* Infinite loop */
    while (1)
    {
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
