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

#include "can_parser.h"
#include "wizchip_conf.h"
#include "w5x00_spi.h"
#include "can.h" 
#include "loopback.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/irq.h"
#include "hardware/structs/pio.h"
#include "RP2040.h"
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (125 * 1000)

/* Buffer */
#define BUF_MAX_SIZE (1024 * 2)

/* Can */
#define CAN_PIO_INDEX       0           // do not use PIO1 (using in w5x00)
#define CAN_BITRATE         100000      // 125kbps
#define CAN_RX_PIN          4
#define CAN_TX_PIN          5

//#define CAN_DEBUG

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
static struct can2040 cbus;

typedef struct _CAN_RX_RBUF
{
    bool        rx_flag;
    uint32_t    push_idx;
    uint32_t    pop_idx;
    struct can2040_msg msg_buf[BUF_MAX_SIZE];
} CAN_RX_RBUF;

static volatile CAN_RX_RBUF can_rx_rbuf; 
static critical_section_t can_cris_sec;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

static void push_rbuf(struct can2040_msg *msg);
static int pop_rbuf(struct can2040_msg *msg);

static int can_initialize();
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg);
static void PIOx_IRQHandler(void);
static int is_pio_in_use(uint32_t pio_num) ;

void dump_can_msg();
void send_can_msg();
/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */

int main()
{
    /* Initialize */
    int retval = 0;
    char input_buf[64];

    set_clock_khz();

    stdio_init_all();

    if(can_initialize() < 0)
    {
        printf("CAN setup failed...\n");
        while(1){}
    }

    can2040_start(&cbus, clock_get_hz(clk_sys), CAN_BITRATE, CAN_RX_PIN, CAN_TX_PIN);

    while(1)
    {   
        printf("\r\n==========================================================\r\n");
        printf("                  CAN UTILS MENU \r\n");
        printf("\r\n==========================================================\r\n");
        printf(" 1 - Dump Message \r\n");
        printf(" 2 - Send Message \r\n");

        memset(input_buf, 0x0, 64);
        printf("\nEnter your choice : ");

        getInputString(input_buf);
        
        if(strcmp(input_buf, "1") == 0)
        {
            dump_can_msg();
        }
        else if(strcmp(input_buf, "2") == 0)
        {
            send_can_msg();
        }
        else
        {
            printf("Input error...\n");
            continue;
        }

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

static void push_rbuf(struct can2040_msg *msg)
{
    critical_section_enter_blocking(&can_cris_sec);

    memcpy(&can_rx_rbuf.msg_buf[can_rx_rbuf.push_idx], msg, sizeof(struct can2040_msg));
    
    if(can_rx_rbuf.push_idx < BUF_MAX_SIZE)
        can_rx_rbuf.push_idx++;
    else    
        can_rx_rbuf.push_idx = 0;

    critical_section_exit(&can_cris_sec);
}

static int pop_rbuf(struct can2040_msg *msg)
{
    if(can_rx_rbuf.pop_idx == can_rx_rbuf.push_idx)
        return -1;

    critical_section_enter_blocking(&can_cris_sec);
    memcpy(msg, &can_rx_rbuf.msg_buf[can_rx_rbuf.pop_idx], sizeof(struct can2040_msg));
    
    if(can_rx_rbuf.pop_idx < BUF_MAX_SIZE)
        can_rx_rbuf.pop_idx++;
    else    
        can_rx_rbuf.pop_idx = 0;

    critical_section_exit(&can_cris_sec);
    return 0;
}

static int can_initialize()
{
    if (is_pio_in_use(CAN_PIO_INDEX)) {
        printf("Error: PIO block %u is already in use\n", CAN_PIO_INDEX);
        return -1;
    }

    can2040_setup(&cbus, CAN_PIO_INDEX);
    can2040_callback_config(&cbus, can2040_cb);

    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    //irq_set_priority(PIO0_IRQ_0_IRQn, 1);
    irq_set_enabled(PIO0_IRQ_0_IRQn, true);

    critical_section_init(&can_cris_sec);

    return 0;
}

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    if (notify == CAN2040_NOTIFY_RX) 
    {
        if(can_rx_rbuf.rx_flag == true)
            push_rbuf(msg);

#ifdef CAN_DEBUG        
        printf("Callback RX: ID=0x%08X DLC=%d Data=", msg->id, msg->dlc);
        for (int i = 0; i < msg->dlc; i++) {
            printf("0x%02X ", msg->data[i]);
        }
        printf("\n");
#endif
    }
    else if(notify == CAN2040_NOTIFY_TX) 
    {
#ifdef CAN_DEBUG      
        printf("Callback TX\n");
#endif
    }
    else {
#ifdef CAN_DEBUG  
        printf("Callback error...\n");
#endif
    }
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

static int is_pio_in_use(uint32_t pio_num) 
{
    pio_hw_t *pio_hw;

    if (pio_num == 0) {
        pio_hw = pio0_hw;
    } else if (pio_num == 1) {
        pio_hw = pio1_hw;
#if IS_RP2350
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

void dump_can_msg()
{
    struct can2040_msg dump_msg;
    uint32_t idx = 0;
    char buf[64];
    int ret = 0;

    can_rx_rbuf.push_idx = 0;
    can_rx_rbuf.pop_idx = 0;
    memset(can_rx_rbuf.msg_buf, 0x0, BUF_MAX_SIZE * sizeof(struct can2040_msg));
    can_rx_rbuf.rx_flag = true;

    printf("If you want to exit press Ctrl + C...\n");

    do {
        buf[0] = getchar_timeout_us(100);

        if(buf[0] == 0x03)      // Ctrl + C
            break;
        
        ret = pop_rbuf(&dump_msg);

        if(ret < 0)
            continue;

        printf("\t0X%x\t%lu\t", dump_msg.id, dump_msg.dlc);

        for(idx = 0; idx < dump_msg.dlc; idx++)
        {
            printf("0x%x ", dump_msg.data[idx]);
        }
        printf("\n");

    } while(1);

    can_rx_rbuf.rx_flag = false;
    can_rx_rbuf.push_idx = 0;
    can_rx_rbuf.pop_idx = 0;
    memset(can_rx_rbuf.msg_buf, 0x0, BUF_MAX_SIZE * sizeof(struct can2040_msg));
}

void send_can_msg()
{
    char buf[64];
    int buf_len = 0;
    struct can2040_msg tx_msg;
    char *p_id_token = NULL, *p_data_token = NULL;

    printf("Usage: <canid>#<candata>\n");
    printf("   ex) 123#12 34 56 78\n");
    printf("\nIf you want to stop press Q...\n\n");

    do {
        printf("> ");

        buf_len = getInputString(buf);

        if( (buf[0] == 'q') || (buf[0] == 'Q') )
            break;

        memset(&tx_msg, 0x0, sizeof(struct can2040_msg));
        p_id_token = NULL;
        p_data_token = NULL;

        p_id_token = custom_strtok(buf, "#");

        if(p_id_token == NULL)
        {
            printf("string format error...\n\n");
            continue;
        }

        tx_msg.id = string_to_hex32(p_id_token);

        do {
            p_data_token = custom_strtok(NULL, " ");
            if(p_data_token == NULL) 
                break;

            if(tx_msg.dlc >= 8)
                break;

            tx_msg.data[tx_msg.dlc] = string_to_hex32(p_data_token);
            tx_msg.dlc++;
        } while(1);

        if( (p_data_token==NULL) && (tx_msg.dlc == 0) )
        {
            printf("string format error...\n\n");
            continue;
        } 

        if(tx_msg.id > 0x7FF)
        {
            printf("ID must be < 0x800\n");
            continue;
        }

#ifdef CAN_DEBUG        
        printf("TX: ID=0x%X DLC=%d Data=", tx_msg.id, tx_msg.dlc);
        for (int i = 0; i < tx_msg.dlc; i++) {
            printf("0x%02X ", tx_msg.data[i]);
        }
        printf("\n");
#endif

        if (can2040_transmit(&cbus, &tx_msg) < 0) {
#ifdef CAN_DEBUG
            printf("Transmit queue full. Failed to send message.\n\n");
#endif
        } 

    }while(1);
}
