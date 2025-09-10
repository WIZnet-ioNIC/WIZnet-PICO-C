#include <stdio.h>
#include "port_common.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "loopback.h"
#include "socket.h"


/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2) // Send and receive cache size
#define _LOOPBACK_DEBUG_

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


static uint8_t ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint16_t local_port = 8000; // Local port


int32_t loopback_tcps_multi_socket(uint8_t *buf, uint16_t port);

int main() {

    /*mcu init*/
    stdio_init_all(); // Initialize the main control peripheral.
    sleep_ms(3000);
    wizchip_spi_initialize();
    wizchip_cris_initialize();
    wizchip_reset();
    wizchip_initialize(); // spi initialization
    wizchip_check();

    network_initialize(g_net_info);

    print_network_information(g_net_info); // Read back the configuration information and print it

    while (true) {
        loopback_tcps_multi_socket(ethernet_buf, local_port);
    }
}


int8_t sn = 0;
int32_t loopback_tcps_multi_socket(uint8_t *buf, uint16_t port) {
    int32_t ret;
    uint16_t size = 0, sentsize = 0;
    uint8_t destip[4];
    uint16_t destport;
    uint16_t multi_socket_port = port + sn ;
    // printf("socket %d : status %d\r\n",sn,getSn_SR(sn));
    switch (getSn_SR(sn)) {
    case SOCK_ESTABLISHED:
        if (getSn_IR(sn) & Sn_IR_CON) {
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);

            printf("%d:Connected - %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
            setSn_IR(sn, Sn_IR_CON);
        }
        if ((size = getSn_RX_RSR(sn)) > 0) { // Don't need to check SOCKERR_BUSY because it doesn't not occur.
            if (size > ETHERNET_BUF_MAX_SIZE) {
                size = ETHERNET_BUF_MAX_SIZE;
            }
            ret = recv(sn, buf, size);
            if (ret <= 0) {
                return ret;    // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
            }
            size = (uint16_t)ret;
            buf[size] = 0x00;

            sentsize = 0;
            while (size != sentsize) {
                ret = send(sn, buf + sentsize, size - sentsize);
                if (ret < 0) {
                    close(sn);
                    return ret;
                }
                sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
            }
            getSn_DIPR(sn, destip);
            destport = getSn_DPORT(sn);
            printf("socket%d from:%d.%d.%d.%d port: %d  message:%s\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport, buf);
        }
        break;
    case SOCK_CLOSE_WAIT:
#ifdef _LOOPBACK_DEBUG_
        // printf("%d:CloseWait\r\n",sn);
#endif
        if ((ret = disconnect(sn)) != SOCK_OK) {
            return ret;
        }
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Socket Closed\r\n", sn);
#endif
        break;
    case SOCK_INIT:
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Listen, TCP server loopback, port [%d]\r\n", sn, multi_socket_port);
#endif
        if ((ret = listen(sn)) != SOCK_OK) {
            return ret;
        }
        break;
    case SOCK_CLOSED:
#ifdef _LOOPBACK_DEBUG_
        // printf("%d:TCP server loopback start\r\n",sn);
#endif
        if ((ret = socket(sn, Sn_MR_TCP, multi_socket_port, Sn_MR_ND)) != sn) {
            return ret;
        }
#ifdef _LOOPBACK_DEBUG_
        // printf("%d:Socket opened\r\n",sn);
#endif
        break;
    default:
        break;
    }

    if (sn >= (_WIZCHIP_SOCK_NUM_  - 1)) {
        sn = 0;
    } else {
        sn ++;
    }

    return 1;
}