# How to Test CAN Example

Reference CAN driver code is based on [link-can_driver].


## Step 1: Prepare software

The following serial terminal programs are required for CAN example test, download and install from below links.

- [**Tera Term**][link-tera_term]
- [**Hercules**][link-hercules]
- [**Klipper**][link-klipper]


## Step 2: Prepare hardware

If you are using W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2, you can skip '1. Combine...'

1. Combine WIZnet Ethernet HAT with Raspberry Pi Pico.

2. Connect ethernet cable to WIZnet Ethernet HAT, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 ethernet port.

3. Connect Raspberry Pi Pico, W5100S-EVB-Pico or W5500-EVB-Pico to desktop or laptop using 5 pin micro USB cable. W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 require a USB Type-C cable.

4. Connect as shown in the diagram below, to test the CAN example.

![][link-hardware_wiring]

## Step 3: Prepare software

By installing the following tools, you can test and debug the CAN example code. (This tool is for Linux)

1. Download Klipper code
```cpp
# Linux
$ git clone https://github.com/Klipper3d/klipper
$ sudo apt-get update && sudo apt-get install build-essential libncurses-dev libusb-dev libnewlib-arm-none-eabi gcc-arm-none-eabi binutils-arm-none-eabi libusb-1.0 pkg-config
```

2. Build USB to CAN bus code and Flashing to board

```cpp
# Linux
$ make menuconfig
```

![][link-menuconfig_for_test_tools]

```cpp
# Linux
$ make
$ make flash FLASH_DEVICE=2e8a:0003
```

3. Install can-utils tools

```cpp
# Linux
$ sudo apt-get install can-utils
$ sudo ip link set can0 up type can bitrate 1000000
```
If the tools for the test board are installed correctly, the result will appear as shown in the diagram below.

![][link-test_board_is_running]


## Step 4: Setup CAN Example

To test the CAN example, minor settings shall be done in code.

Setup CAN Configuration and pin in example in 'WIZnet-PICO-C/examples/can/' directory.

```cpp
/* Can */
#define CAN_PIO_INDEX       0           // !! do not use PIO1 (using in w5x00) !!
#define CAN_BITRATE         100000      // 100kbps
#define CAN_RX_PIN          4
#define CAN_TX_PIN          5
```


## Step 5: Build

1. After completing the CAN example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, '{example_name}.uf2' is generated in 'WIZnet-PICO-C/build/examples/can/{example_folder}' directory.



## Step 6: Upload and Run

1. While pressing the BOOTSEL button of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 power on the board, the USB mass storage 'RPI-RP2' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop '{example_name}.uf2' onto the USB mass storage device 'RPI-RP2'.

3. Connect to the serial COM port of Raspberry Pi Pico, W5100S-EVB-Pico, W5500-EVB-Pico, W55RP20-EVB-Pico, W5100S-EVB-Pico2 or W5500-EVB-Pico2 with Tera Term.

![][link-connect_to_serial_com_port]

4. You can verify if it operates as follows.

```cpp
# Linux
$ candump can0
$ cansend can0 123#1234567887654321
```

![][link-can_message]

Using a logical analyzer to measure the signal of GPIO4, GPIO5 has the following results.

![][link-can_tx_signal]

![][link-can_rx_signal]


<!--
Link
-->

[link-can_driver]: https://github.com/KevinOConnor/can2040
[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-hercules]: https://www.hw-group.com/software/hercules-setup-utility
[link-klipper]: https://www.klipper3d.org/
[link-hardware_wiring]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/hardware_wiring.png
[link-menuconfig_for_test_tools]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/menuconfig_for_test_tools.png
[link-test_board_is_running]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/test_board_is_running.png
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/connect_to_serial_com_port.png
[link-can_message]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_message.png
[link-can_tx_signal]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_tx_signal.png
[link-can_rx_signal]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-C/blob/main/static/images/can/can_can_rx_signalmessage.png