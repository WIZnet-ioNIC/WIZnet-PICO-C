# How to Test MQTT Publish & Subscribe Example



## Step 1: Prepare software

The following serial terminal program and MQTT broker are required for MQTT Publish & Subscribe example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Mosquitto**][link-mosquitto]



## Step 2: Prepare hardware

If you are using WIZnet's PICO board, you can skip '1. Combine...'

1. If you are using WIZnet Ethernet HAT, Combine it with Raspberry Pi Pico.

2. Connect ethernet cable to your PICO board ethernet port.

3. Connect your PICO board to desktop or laptop using USB cable. 



## Step 3: Setup MQTT Publish & Subscribe Example

To test the MQTT Publish & Subscribe example, minor settings shall be done in code.

1. Setup SPI port and pin in 'wizchip_spi.h' in 'WIZnet-PICO-C/port/ioLibrary_Driver/' directory.

Setup the SPI interface you use.

### For **W55RP20-EVB-PICO**:
If you are using the **W55RP20-EVB-PICO**, enable `USE_PIO` and configure as follows:

```cpp
#if (DEVICE_BOARD_NAME == W55RP20_EVB_PICO)

#define USE_PIO

#define PIN_SCK   21
#define PIN_MOSI  23
#define PIN_MISO  22
#define PIN_CS    20
#define PIN_RST   25
#define PIN_IRQ   24

```

---

### For **W6300-EVB-PICO** or **W6300-EVB-PICO2**:
If you are using the **W6300-EVB-PICO** or **W6300-EVB-PICO2**, use the following pinout and SPI clock divider configuration:

```cpp
#elif (DEVICE_BOARD_NAME == W6300_EVB_PICO || DEVICE_BOARD_NAME == W6300_EVB_PICO2)
#define USE_PIO

#define PIO_IRQ_PIN             15
#define PIO_SPI_SCK_PIN         17
#define PIO_SPI_DATA_IO0_PIN    18
#define PIO_SPI_DATA_IO1_PIN    19
#define PIO_SPI_DATA_IO2_PIN    20
#define PIO_SPI_DATA_IO3_PIN    21
#define PIN_CS                  16
#define PIN_RST                 22


```

---

### For other generic SPI boards
If you are not using any of the above boards, you can fall back to a default SPI configuration:

```cpp
#else

#define SPI_PORT spi0

#define SPI_SCK_PIN  18
#define SPI_MOSI_PIN 19
#define SPI_MISO_PIN 16
#define SPI_CS_PIN   17
#define RST_PIN      20

#endif
```

Make sure you are **not defining `USE_PIO`** in your setup when using DMA:

```cpp
// #define USE_PIO
```

2. Setup network configuration such as IP in 'wizchip_mqtt_publish_subscribe.c' which is the MQTT Publish & Subscribe example in 'WIZnet-PICO-C/examples/mqtt/publish_subscribe/' directory.

Setup IP and other network settings to suit your network environment.

```cpp
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
```

3. Setup MQTT configuration in wizchip_mqtt_publish_subscribe.c' in 'WIZnet-PICO-C/examples/mqtt/publish_subscribe/' directory.

In the MQTT configuration, the broker IP is the IP of your desktop or laptop where broker will be created.

```cpp
/* Port */
#define PORT_MQTT 1883

/* MQTT */
#define MQTT_CLIENT_ID "rpi-pico"
#define MQTT_USERNAME "wiznet"
#define MQTT_PASSWORD "0123456789"
#define MQTT_PUBLISH_TOPIC "publish_topic"
#define MQTT_PUBLISH_PAYLOAD "Hello, World!"
#define MQTT_PUBLISH_PERIOD (1000 * 10) // 10 seconds
#define MQTT_SUBSCRIBE_TOPIC "subscribe_topic"
#define MQTT_KEEP_ALIVE 60 // 60 milliseconds

static uint8_t g_mqtt_broker_ip[4] = {192, 168, 11, 3};
```



## Step 4: Build

1. After completing the MQTT Publish & Subscribe example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'wizchip_mqtt_publish_subscribe.uf2' is generated in 'WIZnet-PICO-C/build/examples/mqtt/publish_subscribe/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of the Pico power on the board, the USB mass storage 'RPI-RP2' or 'RP2350' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'wizchip_mqtt_publish_subscribe.uf2' onto the USB mass storage device 'RPI-RP2' or 'RP2350'.

3. Connect to the serial COM port of the pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Run Mosquitto to be used as the broker.

![][link-run_mosquitto]

5. Create broker using Mosquitto by executing the following command. If the broker is created normally, the broker's IP is the current IP of your desktop or laptop, and the port is 1883 by default.

```cpp
mosquitto -c mosquitto.conf -v
```

![][link-create_mqtt_broker_using_mosquitto]

6. Reset your board.

7. If the MQTT Publish & Subscribe example works normally on the pico, you can see the network information of the pico, connecting to the broker, subscribing to the subscribe topic and publishing the message.

![][link-see_network_information_of_raspberry_pi_pico_connecting_to_broker_subscribing_to_subscribe_topic_and_publishing_message_1]

![][link-see_network_information_of_raspberry_pi_pico_connecting_to_broker_subscribing_to_subscribe_topic_and_publishing_message_2]



## Appendix

- In Mosquitto versions earlier than 2.0 the default is to allow clients to connect without authentication. In 2.0 and up, you must choose your authentication options explicitly before clients can connect. Therefore, if you are using version 2.0 or later, refer to following link to setup 'mosquitto.conf' in the directory where Mosquitto is installed.

    - [**Authentication Methods**][link-authentication_methods]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-mosquitto]: https://mosquitto.org/download/
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/mqtt/publish_subscribe/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/mqtt/publish_subscribe/connect_to_serial_com_port.png
[link-run_mosquitto]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/mqtt/publish/run_mosquitto.png
[link-create_mqtt_broker_using_mosquitto]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/mqtt/publish_subscribe/create_mqtt_broker_using_mosquitto.png
[link-see_network_information_of_raspberry_pi_pico_connecting_to_broker_subscribing_to_subscribe_topic_and_publishing_message_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/mqtt/publish_subscribe/see_network_information_of_raspberry_pi_pico_connecting_to_broker_subscribing_to_subscribe_topic_and_publishing_message_1.png
[link-see_network_information_of_raspberry_pi_pico_connecting_to_broker_subscribing_to_subscribe_topic_and_publishing_message_2]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/mqtt/publish_subscribe/see_network_information_of_raspberry_pi_pico_connecting_to_broker_subscribing_to_subscribe_topic_and_publishing_message_2.png
[link-authentication_methods]: https://mosquitto.org/documentation/authentication-methods/
