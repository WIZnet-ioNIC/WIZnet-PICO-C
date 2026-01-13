#ifndef	__HTTPHANDLER_H__
#define	__HTTPHANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void make_json_devinfo(uint8_t * buf, uint16_t * len);
uint8_t set_devinfo(uint8_t * uri);

#ifdef __cplusplus
}
#endif

#endif	/* end of __HTTPHANDLER_H__ */