#ifndef _CANHANDLER_H_
#define _CANHANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include "port_common.h"
#include "can.h"
#include "w5x00_can_config.h"

#define CAN_PIO_INDEX       0
#define CAN_RX_PIN          4
#define CAN_TX_PIN          5

#define DATA_BUF_MAX_SIZE (1024 * 2)

#define CAN_ERR_FORMAT          -1
#define CAN_ERR_ID_OVERFLOW     -2

typedef struct _CAN_RX_RBUF {
    bool        rx_flag;
    uint32_t    push_idx;
    uint32_t    pop_idx;
    struct can2040_msg msg_buf[DATA_BUF_MAX_SIZE];
} CAN_RX_RBUF;

CanConfig* get_CanConfig_pointer(void);
void get_default_can_config(CanConfig* p_can_config);

void push_rbuf(struct can2040_msg *msg);
int pop_rbuf(struct can2040_msg *msg);

int can_initialize(CanConfig *p_can_config);
int can_restart(CanConfig *p_can_config);
void PIOx_IRQHandler(void);
int send_can_msg(char *msg_buf);

void printCanConfig(CanConfig *p_can_config);

#endif /* _CANHANDLER_H_ */