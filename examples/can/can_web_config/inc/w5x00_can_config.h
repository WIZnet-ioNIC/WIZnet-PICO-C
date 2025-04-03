
#ifndef _W5X00_CAN_CONFIG_H_
#define _W5X00_CAN_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include "port_common.h"
//#include "can.h"
// #include "canHandler.h"

typedef enum _ETH_MODE {
  ETH_TCP_SERVER = 0,
  ETH_TCP_CLIENT,

  ETH_MODE_MAX
} ETH_MODE;

typedef enum _BDRATE {
  BDRATE_125,
  BDRATE_250,
  BDRATE_500,

  BDRATE_MAX
} BDRATE;

typedef struct _CanConfig {
  ETH_MODE      eth_mode;
  BDRATE        baudrate;
} CanConfig;

#endif /* _W5X00_CAN_CONFIG_H_ */
