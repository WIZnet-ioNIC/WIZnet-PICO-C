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
#include <string.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"

#include "mqtt_interface.h"
#include "MQTTClient.h"

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
#define SOCKET_MQTT 0

/* Port */
#define PORT_MQTT 1883

/* Timeout */
#define DEFAULT_TIMEOUT 1000 // 1 second

/* MQTT */
#define MQTT_CLIENT_ID "rpi-pico"
#define MQTT_USERNAME "wiznet"
#define MQTT_PASSWORD "0123456789"
#define MQTT_PUBLISH_TOPIC "publish_topic"
#define MQTT_PUBLISH_PAYLOAD "Hello, World!"
#define MQTT_PUBLISH_PERIOD (1000 * 10) // 10 seconds
#define MQTT_KEEP_ALIVE 60              // 60 milliseconds

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

/* MQTT */
static uint8_t g_mqtt_send_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_mqtt_recv_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_mqtt_broker_ip[4] = {192, 168, 11, 3};
static Network g_mqtt_network;
static MQTTClient g_mqtt_client;
static MQTTPacket_connectData g_mqtt_packet_connect_data = MQTTPacket_connectData_initializer;
static MQTTMessage g_mqtt_message;

/* Timer  */
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
    int32_t retval = 0;
    uint32_t start_ms = 0;
    uint32_t end_ms = 0;

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

    NewNetwork(&g_mqtt_network, SOCKET_MQTT);

    retval = ConnectNetwork(&g_mqtt_network, g_mqtt_broker_ip, PORT_MQTT);

    if (retval != 1) {
        printf(" Network connect failed\n");

        while (1)
            ;
    }

    /* Initialize MQTT client */
    MQTTClientInit(&g_mqtt_client, &g_mqtt_network, DEFAULT_TIMEOUT, g_mqtt_send_buf, ETHERNET_BUF_MAX_SIZE, g_mqtt_recv_buf, ETHERNET_BUF_MAX_SIZE);

    /* Connect to the MQTT broker */
    g_mqtt_packet_connect_data.MQTTVersion = 3;
    g_mqtt_packet_connect_data.cleansession = 1;
    g_mqtt_packet_connect_data.willFlag = 0;
    g_mqtt_packet_connect_data.keepAliveInterval = MQTT_KEEP_ALIVE;
    g_mqtt_packet_connect_data.clientID.cstring = MQTT_CLIENT_ID;
    g_mqtt_packet_connect_data.username.cstring = MQTT_USERNAME;
    g_mqtt_packet_connect_data.password.cstring = MQTT_PASSWORD;

    retval = MQTTConnect(&g_mqtt_client, &g_mqtt_packet_connect_data);

    if (retval < 0) {
        printf(" MQTT connect failed : %d\n", retval);

        while (1)
            ;
    }

    printf(" MQTT connected\n");

    /* Configure publish message */
    g_mqtt_message.qos = QOS0;
    g_mqtt_message.retained = 0;
    g_mqtt_message.dup = 0;
    g_mqtt_message.payload = MQTT_PUBLISH_PAYLOAD;
    g_mqtt_message.payloadlen = strlen(g_mqtt_message.payload);

    start_ms = millis();

    /* Infinite loop */
    while (1) {
        end_ms = millis();

        if (end_ms > start_ms + MQTT_PUBLISH_PERIOD) {
            /* Publish */
            retval = MQTTPublish(&g_mqtt_client, MQTT_PUBLISH_TOPIC, &g_mqtt_message);

            if (retval < 0) {
                printf(" Publish failed : %d\n", retval);

                while (1)
                    ;
            }

            printf(" Published\n");

            start_ms = millis();
        }

        if ((retval = MQTTYield(&g_mqtt_client, g_mqtt_packet_connect_data.keepAliveInterval)) < 0) {
            printf(" Yield error : %d\n", retval);

            while (1)
                ;
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

    MilliTimer_Handler();
}

static time_t millis(void) {
    return g_msec_cnt;
}
