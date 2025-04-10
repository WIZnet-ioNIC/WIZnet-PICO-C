/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIZCHIP_GPIO_IRQ_H_
#define _WIZCHIP_GPIO_IRQ_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* GPIO */
#if (_WIZCHIP_ <= W5500)
#define PIN_INT 21
#elif (_WIZCHIP_ == W6100)
#define PIN_INT 22
#elif (_WIZCHIP_ == W6300)
#define PIN_INT 15
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* GPIO */
/*! \brief Initialize wizchip gpio interrupt callback function
 *  \ingroup wizchip_gpio_irq
 *
 *  Add a wizchip interrupt callback.
 *
 *  \param socket socket number
 *  \param callback the gpio interrupt callback function
 */
void wizchip_gpio_interrupt_initialize(uint8_t socket, void (*callback)(void));

/*! \brief Assign gpio interrupt callback function
 *  \ingroup wizchip_gpio_irq
 *
 *  GPIO interrupt callback function.
 *
 *  \param gpio Which GPIO caused this interrupt
 *  \param events Which events caused this interrupt. See \ref gpio_set_irq_enabled for details.
 */
static void wizchip_gpio_interrupt_callback(uint gpio, uint32_t events);

#endif /* _WIZCHIP_GPIO_IRQ_H_ */
