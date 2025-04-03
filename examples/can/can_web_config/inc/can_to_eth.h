#ifndef	__HTTPHANDLER_H__
#define	__HTTPHANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "w5x00_can_config.h"

/* Socket */
#define SOCKET_TCP  0

int32_t can_to_eth(ETH_MODE mode);
int32_t reset_eth();

#ifdef __cplusplus
}
#endif

#endif	/* end of __HTTPHANDLER_H__ */