# How to Test TCP Client over SSL Example



## Step 1: Prepare software

The following serial terminal program and SSL server are required for TCP Client over SSL example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**OpenSSL**][link-openssl]



## Step 2: Prepare hardware

If you are using WIZnet's PICO board, you can skip '1. Combine...'

1. If you are using WIZnet Ethernet HAT, Combine it with Raspberry Pi Pico.

2. Connect ethernet cable to your PICO board ethernet port.

3. Connect your PICO board to desktop or laptop using USB cable. 



## Step 3: Setup TCP Client over SSL Example

To test the TCP Client over SSL example, minor settings shall be done in code.

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

2. Setup network configuration such as IP in 'wizchip_tcp_client_over_ssl.c' which is the TCP Client over SSL example in 'WIZnet-PICO-C/examples/tcp_client_over_ssl/' directory.



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

3. Setup TCP Client over SSL configuration in 'wizchip_tcp_client_over_ssl.c' in 'WIZnet-PICO-C/examples/tcp_client_over_ssl/' directory.

In the TCP client over SSL configuration, the target IP is the IP of your desktop or laptop where SSL server will be created.

```cpp
/* Port */
#define PORT_SSL 443

static uint8_t g_ssl_target_ip[4] = {192, 168, 11, 3};
```

In order to change SSL settings, modify 'ssl_config.h' located in the same directory with TCP Client over SSL example.



## Step 4: Build

1. After completing the TCP Client over SSL example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'wizchip_tcp_client_over_ssl.uf2' is generated in 'WIZnet-PICO-C/build/examples/tcp_client_over_ssl/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of the Pico power on the board, the USB mass storage 'RPI-RP2' or 'RP2350' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'wizchip_tcp_client_over_ssl.uf2' onto the USB mass storage device 'RPI-RP2' or 'RP2350'.

3. Connect to the serial COM port of the pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Run OpenSSL to be used as the SSL server.

![][link-run_openssl]

5. Create SSL server using openSSL by executing the following command. If the SSL server is created normally, the SSL server's IP is the current IP of your desktop or laptop, and the port is 443 by default.

```cpp
/* Setup the SSL server */
// create the private key
genrsa -des3 -out [key name].key 2048
// create the CSR(required for certificate signing request)
req -new -key [key name].key -out [csr name].csr
// create the certificate
x509 -req -days [days] -in [csr name].csr -signkey [key name].key -out [crt name].crt

// e.g.
genrsa -des3 -out server.key 2048
req -new -key server.key -out server.csr
x509 -req -days 365 -in server.csr -signkey server.key -out server.crt

/* Run the SSL server */
s_server -accept [port] -cert [crt name].crt -key [key name].key

// e.g.
s_server -accept 443 -cert server.crt -key server.key
```

![][link-create_ssl_server_using_openssl]

6. Reset your board.

7. If the TCP Client over SSL example works normally on the pico, you can see the network information of the pico, connecting to the SSL server and sending the message.

![][link-see_network_information_of_raspberry_pi_pico_connecting_to_ssl_server_and_sending_message_1]

![][link-see_network_information_of_raspberry_pi_pico_connecting_to_ssl_server_and_sending_message_2]

8. Once connected if you send data to the pico from the SSL server, you should be able to receive the sent message on it

![][link-receive_sent_message_1]

![][link-receive_sent_message_2]



## Appendix

- mbedTLS library was ported to use SSL, please refer to following link to find version of ported mbed TLS.

	- [**mbed TLS**][link-mbed_tls]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-openssl]: https://www.openssl.org/source/
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/connect_to_serial_com_port.png
[link-run_openssl]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/run_openssl.png
[link-create_ssl_server_using_openssl]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/create_ssl_server_using_openssl.png
[link-see_network_information_of_raspberry_pi_pico_connecting_to_ssl_server_and_sending_message_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/see_network_information_of_raspberry_pi_pico_connecting_to_ssl_server_and_sending_message_1.png
[link-see_network_information_of_raspberry_pi_pico_connecting_to_ssl_server_and_sending_message_2]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/see_network_information_of_raspberry_pi_pico_connecting_to_ssl_server_and_sending_message_2.png
[link-receive_sent_message_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/receive_sent_message_1.png
[link-receive_sent_message_2]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/tcp_client_over_ssl/receive_sent_message_2.png
[link-mbed_tls]: https://github.com/Mbed-TLS/mbedtls/tree/8df2f8e7b9c7bb9390ac74bb7bace27edca81a2b
