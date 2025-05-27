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
#include "can.h" 
#include "loopback.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/irq.h"
#include "hardware/structs/pio.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
//#define CAN_DEBUG

/* Clock */
#define PLL_SYS_KHZ (125 * 1000)

/* Can */
#define CAN_PIO_INDEX       0           // !! do not use PIO1 (using in w5x00) !!
#define CAN_BITRATE         100000      // 100kbps
#define CAN_RX_PIN          4
#define CAN_TX_PIN          5

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
static struct can2040 cbus;

static struct can2040_msg latest_msg = {};
static volatile uint32_t latest_notify = 0;
static volatile bool new_message = false;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
static void PIOx_IRQHandler(void);
static int can_initialize();
static int is_pio_in_use(uint32_t pio_num);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

int main()
{
    /* Initialize */
    int retval = 0;
    uint64_t tx_msg_64 = 0;
    struct can2040_msg tx_msg;

    set_clock_khz();

    stdio_init_all();
    sleep_ms(3000);
    if(can_initialize() < 0)
    {
        printf("CAN setup failed...\n");
        while(1){}
    }

    printf("CAN setup success\n");
    
    can2040_start(&cbus, clock_get_hz(clk_sys), CAN_BITRATE, CAN_RX_PIN, CAN_TX_PIN);

    while(1)
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

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    new_message = true;
    latest_notify = notify;
    latest_msg = *msg;

    if (notify == CAN2040_NOTIFY_RX) 
    {
#ifdef CAN_DEBUG
        printf("Callback RX: ID=0x%08X DLC=%d Data=", msg->id, msg->dlc);
        for (int i = 0; i < msg->dlc; i++) {
            printf("0x%02X ", msg->data[i]);
        }
        printf("\n");
#endif
        if (can2040_transmit(cd, msg) < 0) {
#ifdef CAN_DEBUG
            printf("Transmit queue full. Failed to send message.\n\n");
#endif
        } 
    }
    else if(notify == CAN2040_NOTIFY_TX) 
    {
#ifdef CAN_DEBUG
        printf("Callback TX\n");
#endif
    }
    else {
#ifdef CAN_DEBUG
        printf("Callback error!\n");
#endif
    }
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

static int can_initialize()
{
    if (is_pio_in_use(CAN_PIO_INDEX)) {
        printf("Error: PIO block %u is already in use\n", CAN_PIO_INDEX);
        return -1;
    }

    can2040_setup(&cbus, CAN_PIO_INDEX);
    can2040_callback_config(&cbus, can2040_cb);

    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, 1);
    irq_set_enabled(PIO0_IRQ_0, true);

    return 0;
}

static int is_pio_in_use(uint32_t pio_num) {
    pio_hw_t *pio_hw;

    if (pio_num == 0) {
        pio_hw = pio0_hw;
    } else if (pio_num == 1) {
        pio_hw = pio1_hw;
#ifdef PICO_RP2350
    } else if (pio_num == 2) {
        pio_hw = pio2_hw;
#endif
    } else {
        printf("Invalid PIO number: %u\n", pio_num);
        return -1;
    }

    for (int sm = 0; sm < 4; sm++) {
        if ((pio_hw->ctrl & (1 << (PIO_CTRL_SM_ENABLE_LSB + sm))) != 0) {
            return 1; 
        }
    }
    return 0;
}