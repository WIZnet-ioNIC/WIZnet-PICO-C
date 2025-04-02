#include <stdio.h>
#include "port_common.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "socket.h"
#include "loopback.h"
#include "multicast.h" // Use multicast

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define SOCKET_ID 0                      // Socket number
#define ETHERNET_BUF_MAX_SIZE (1024 * 2) // Send and receive cache size

wiz_NetInfo net_info = {
    .mac = {0x00, 0x08, 0xdc, 0x16, 0xed, 0x2e},    // Define MAC variables
    .ip = {192, 168, 11, 15},                       // Define IP variables
    .sn = {255, 255, 255, 0},                       // Define subnet variables
    .gw = {192, 168, 11, 1},                        // Define gateway variables
    .dns = {168, 126, 63, 1},                       // Define DNS  variables
    .dhcp = NETINFO_STATIC};                        // Define the DNCP mode

static uint8_t ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t multicast_ip[4] = {224, 0, 0, 5};                       // multicast ip address
static uint16_t multicast_port = 30000;                                 // multicast port

static void set_clock_khz(void);

int main()
{
    set_clock_khz();

    /*mcu init*/
    stdio_init_all();     // Initialize the main control peripheral.
    wizchip_spi_initialize();
	wizchip_cris_initialize();
    wizchip_reset();
    wizchip_initialize(); // spi initialization
    wizchip_check();

    network_initialize(net_info);

    print_network_information(net_info); // Read back the configuration information and print it

    while (true)
    {
        // Multicast receive test
        multicast_recv(SOCKET_ID, ethernet_buf, multicast_ip, multicast_port);
    }
}

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