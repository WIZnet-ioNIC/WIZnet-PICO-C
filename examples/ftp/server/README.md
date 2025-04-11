# How to Test FTP Server Example



## Step 1: Prepare software

The following serial terminal program and FTP client are required for FTP Server example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**WinSCP**][link-winscp]



## Step 2: Prepare hardware

If you are using WIZnet's PICO board, you can skip '1. Combine...'

1. If you are using WIZnet Ethernet HAT, Combine it with Raspberry Pi Pico.

2. Connect ethernet cable to your PICO board ethernet port.

3. Connect your PICO board to desktop or laptop using USB cable. 




## Step 3: Setup FTP Server Example

To test the FTP Server example, minor settings shall be done in code.

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
#elif (DEVICE_BOARD_NAME == W6300_EVB_PICO2)
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

2. Setup network configuration such as IP in 'wizchip_ftp_server.c' which is the FTP Server example in 'WIZnet-PICO-C/examples/ftp/server/' directory.

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



## Step 4: Build

1. After completing the FTP Server example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'wizchip_ftp_server.uf2' is generated in 'WIZnet-PICO-C/build/examples/ftp/server/' directory.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of the Pico power on the board, the USB mass storage 'RPI-RP2' or 'RP2350' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'wizchip_ftp_server.uf2' onto the USB mass storage device 'RPI-RP2' or 'RP2350'.

3. Connect to the serial COM port of the pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the FTP Server example works normally on the pico, you can see the network information of the pico and the FTP server is open.

![][link-see_network_information_of_raspberry_pi_pico_and_open_ftp_server]

6. Run WinSCP to be used as the FTP client.

![][link-run_winscp]

7. Connect to the open FTP server using WinSCP. When connecting to the FTP server, you need to select FTP from the 'File protocol' session and enter IP that was configured in Step 3, the port is 21 by default. After completing the setup, click the 'Login' button.

![][link-enter_ftp_server_ip_or_host_name_and_port]

8. Click 'OK' button without entering the user name and password.

![][link-enter_user_name]

![][link-enter_password]

9. After all actions are completed, you shoud be able to connect to FTP server.

![][link-connect_to_ftp_server_1]

![][link-connect_to_ftp_server_2]

10. Let's try to download file from FTP server to FTP client, you should be able to get results similar to below.

![][link-download_selected_remote_file_to_local_directory_1]

![][link-download_selected_remote_file_to_local_directory_2]

11. Now, let's try to upload file from FTP client to FTP server, you should be able to see the contents of the uploaded file printed in terminal.

![][link-upload_selected_local_file_to_remote_directory_1]

![][link-upload_selected_local_file_to_remote_directory_2]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-winscp]: https://winscp.net/eng/download.php
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/connect_to_serial_com_port.png
[link-see_network_information_of_raspberry_pi_pico_and_open_ftp_server]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/see_network_information_of_raspberry_pi_pico_and_open_ftp_server.png
[link-run_winscp]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/run_winscp.png
[link-enter_ftp_server_ip_or_host_name_and_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/enter_ftp_server_ip_or_host_name_and_port.png
[link-enter_user_name]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/enter_user_name.png
[link-enter_password]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/enter_password.png
[link-connect_to_ftp_server_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/connect_to_ftp_server_1.png
[link-connect_to_ftp_server_2]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/connect_to_ftp_server_2.png
[link-download_selected_remote_file_to_local_directory_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/download_selected_remote_file_to_local_directory_1.png
[link-download_selected_remote_file_to_local_directory_2]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/download_selected_remote_file_to_local_directory_2.png
[link-upload_selected_local_file_to_remote_directory_1]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/upload_selected_local_file_to_remote_directory_1.png
[link-upload_selected_local_file_to_remote_directory_2]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/ftp/server/upload_selected_local_file_to_remote_directory_2.png
