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

#include "socket.h"

/**
    ----------------------------------------------------------------------------------------------------
    Macros
    ----------------------------------------------------------------------------------------------------
*/
/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_IGMP 0

/* Port */
#define PORT_IGMP_LOCAL 5000
#define PORT_IGMP_GROUP 5000

/*
    IGMP version (Sn_MR bit 5, MC)
    W5500 : 0 = IGMPv2, 1 = IGMPv1
    Note : ioLibrary names this bit SF_IGMP_VER2, but setting it selects IGMPv1.
*/
// #define USE_IGMP_V1

/**
    ----------------------------------------------------------------------------------------------------
    Variables
    ----------------------------------------------------------------------------------------------------
*/
/* Network */
static wiz_NetInfo g_net_info = {
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 11, 12},                     // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_STATIC
};

/* Multicast group (avoid 224.0.0.x, link-local groups are not reported by IGMP) */
static uint8_t g_igmp_group_ip[4] = {239, 1, 1, 1}; //{239, 1, 2, 3};

/* IGMP */
static uint8_t g_igmp_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* IGMP */
static int8_t igmp_socket_open(uint8_t sn, uint8_t *group_ip, uint16_t group_port, uint16_t local_port);
static void igmp_socket_close(uint8_t sn);
static void igmp_socket_recv(uint8_t sn, uint8_t *buf);

/**
    ----------------------------------------------------------------------------------------------------
    Main
    ----------------------------------------------------------------------------------------------------
*/
int main() {
    /* Initialize */
    int8_t retval = 0;
    int ch;

    stdio_init_all();

    sleep_ms(3000);

    printf("==========================================================\n");
    printf("Compiled @ %s, %s\n", __DATE__, __TIME__);
    printf("==========================================================\n");

    wizchip_spi_initialize();
    wizchip_cris_initialize();
    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

#if (_WIZCHIP_ == W5500)

    /* Open IGMP socket (W5500 sends IGMP Membership Report on OPEN) */
    if ((retval = igmp_socket_open(SOCKET_IGMP, g_igmp_group_ip, PORT_IGMP_GROUP, PORT_IGMP_LOCAL)) != SOCKET_IGMP) {
        printf(" igmp_socket_open error : %d\n", retval);

        while (1)
            ;
    }

    printf(" Press 'o' to join (open), 'c' to leave (close)\n");

    /* Infinite loop */
    while (1) {
        /* Join / Leave control over USB serial */
        ch = getchar_timeout_us(0);

        if (ch == 'o' && getSn_SR(SOCKET_IGMP) == SOCK_CLOSED) {
            if ((retval = igmp_socket_open(SOCKET_IGMP, g_igmp_group_ip, PORT_IGMP_GROUP, PORT_IGMP_LOCAL)) != SOCKET_IGMP) {
                printf(" igmp_socket_open error : %d\n", retval);
            }
        } else if (ch == 'c' && getSn_SR(SOCKET_IGMP) == SOCK_UDP) {
            igmp_socket_close(SOCKET_IGMP);
        }

        /* IGMP Query is answered by W5500 automatically, just drain multicast data here */
        //igmp_socket_recv(SOCKET_IGMP, g_igmp_buf);
    }
#endif
}

/**
    ----------------------------------------------------------------------------------------------------
    Functions
    ----------------------------------------------------------------------------------------------------
*/
/* IGMP */
static int8_t igmp_socket_open(uint8_t sn, uint8_t *group_ip, uint16_t group_port, uint16_t local_port) {
    uint8_t group_mac[6];
    uint8_t flag = SF_MULTI_ENABLE;
    int8_t retval;

    /* Multicast MAC : 01:00:5E + lower 23 bits of group IP */
    group_mac[0] = 0x01;
    group_mac[1] = 0x00;
    group_mac[2] = 0x5E;
    group_mac[3] = group_ip[1] & 0x7F;
    group_mac[4] = group_ip[2];
    group_mac[5] = group_ip[3];

    /* Sn_DHAR, Sn_DIPR, Sn_DPORT must be set before OPEN */
    setSn_DHAR(sn, group_mac);
    setSn_DIPR(sn, group_ip);
    setSn_DPORT(sn, group_port);

#ifdef USE_IGMP_V1
    flag |= Sn_MR_MC;
#endif

    if ((retval = socket(sn, Sn_MR_UDP, local_port, flag)) != sn) {
        return retval;
    }

    printf(" %d : IGMP socket opened (IGMPv%d), Sn_MR = 0x%02X\n", sn,
#ifdef USE_IGMP_V1
           1,
#else
           2,
#endif
           getSn_MR(sn));
    printf(" %d : Group IP  - %d.%d.%d.%d\n", sn, group_ip[0], group_ip[1], group_ip[2], group_ip[3]);
    printf(" %d : Group MAC - %02X:%02X:%02X:%02X:%02X:%02X\n", sn,
           group_mac[0], group_mac[1], group_mac[2], group_mac[3], group_mac[4], group_mac[5]);
    printf(" %d : Group port - %d, Local port - %d\n", sn, group_port, local_port);

    return retval;
}

static void igmp_socket_close(uint8_t sn) {
    /* W5500 sends IGMP Leave Group (v2) on CLOSE */
    close(sn);

    printf(" %d : IGMP socket closed (Leave)\n", sn);
}

static void igmp_socket_recv(uint8_t sn, uint8_t *buf) {
    int32_t retval;
    uint16_t size;
    uint8_t src_ip[4];
    uint16_t src_port;

    if (getSn_SR(sn) != SOCK_UDP) {
        return;
    }

    if ((size = getSn_RX_RSR(sn)) == 0) {
        return;
    }

    if (size > ETHERNET_BUF_MAX_SIZE) {
        size = ETHERNET_BUF_MAX_SIZE;
    }

    if ((retval = recvfrom(sn, buf, size, src_ip, &src_port)) <= 0) {
        printf(" %d : recvfrom error : %ld\n", sn, retval);

        return;
    }

    printf(" %d : recv %ld bytes from %d.%d.%d.%d:%d\n", sn, retval,
           src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
}
